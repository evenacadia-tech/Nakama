#requires -Version 7.2
<#
NAK-256 - die Beine (a) bis (e), einzeln gefahren, Rohausgabe nach
docs/beweise/roh/NAK-256-beine.txt. Aufruf vom Workspace-Root:

  pwsh -NoProfile -File docs/beweise/roh/NAK-256-beine.ps1 [-Klon <pfad>]

Die Vorher-Seite laeuft in einer lokalen Klonung am Basis-SHA (git clone --local, kein
git worktree - das Cockpit meldet Worktrees als Stoerung): dort liegt an derselben Stelle
nacheinander die Fassung des Basis-SHA (git show) und die neue Fassung aus dem Arbeitsbaum,
damit beide dieselben Plandaten lesen. Bein (d) oeffnet sichtbar fuer wenige Sekunden ein
Windows-Terminal-Fenster „nak256-probe" und ein Konsolenfenster; beide schliessen von selbst,
weil der Plan-Tab an einen Platzhalterprozess gebunden ist. Proben-Anker, Zaehler und
Caches liegen im Temp-Ordner; der echte Ticketanker und das Repo bleiben unberuehrt.
ANSI-Steuerzeichen werden aus den Mitschnitten entfernt, sonst steht alles roh da.
#>
[CmdletBinding()]
param(
    [string]$Klon = (Join-Path ([IO.Path]::GetTempPath()) 'nak256-klon'),
    [string]$Basis = 'bb260a2a54088a20cb25c5ccbfa3a1ca8aff3ad7'
)

$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8

$wurzel = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$roh = Join-Path $wurzel 'docs\beweise\roh\NAK-256-beine.txt'
$beispiel = Join-Path $wurzel 'docs\beweise\roh\NAK-256-statuszeile-beispiel.json'
$ersatzClaude = Join-Path $wurzel 'docs\beweise\roh\NAK-256-ersatz-claude.ps1'
$cockpit = Join-Path $wurzel 'tools\dirigent\cockpit.ps1'
$starter = Join-Path $wurzel 'tools\dirigent\start-dirigent.ps1'
$planTab = Join-Path $wurzel 'tools\dirigent\plan-tab.ps1'
$pwsh = (Get-Process -Id $PID).Path
$probe = Join-Path ([IO.Path]::GetTempPath()) "nak256-probe-$PID"
[void][IO.Directory]::CreateDirectory($probe)
$ankerProbe = Join-Path $probe 'anker.json'
$marker = Join-Path ([IO.Path]::GetTempPath()) 'nakama-dirigent-neustart.marker'
# Der Starter schreibt mit der Konsolen-Codepage (er setzt keine Kodierung), das Cockpit mit UTF-8.
$oem = try { [Text.Encoding]::GetEncoding([Globalization.CultureInfo]::CurrentCulture.TextInfo.OEMCodePage) } catch { $utf8 }

$aus = New-Object System.Collections.Generic.List[string]
$bilanz = New-Object System.Collections.Generic.List[object]

function Schreibe {
    param([string]$Text = '')
    foreach ($zeile in ($Text -split "`r?`n")) { $aus.Add($zeile) }
}

function Pruefe {
    param([string]$Bein, [string]$Zusage, [bool]$Erfuellt)
    $bilanz.Add([pscustomobject]@{ Bein = $Bein; Zusage = $Zusage; Erfuellt = $Erfuellt })
    Schreibe ('PRUEFUNG ({0}) {1}: {2}' -f $Bein, $Zusage, $(if ($Erfuellt) { 'ja' } else { 'NEIN' }))
}

function Umgebung {
    # Jede Probe liest den Proben-Anker statt des echten und startet ohne COLUMNS.
    param([hashtable]$Zusatz = @{})
    $werte = @{ NAKAMA_DIRIGENT_ANKER = $ankerProbe; COLUMNS = $null }
    foreach ($schluessel in $Zusatz.Keys) { $werte[$schluessel] = $Zusatz[$schluessel] }
    return $werte
}

function Invoke-Probe {
    param(
        [string[]]$Argumente,
        [string]$Arbeitsordner = $wurzel,
        [hashtable]$Umgebung = @{},
        [string]$Eingabe = $null,
        [int]$FristSekunden = 120,
        [Text.Encoding]$Kodierung = $utf8
    )
    $psi = [Diagnostics.ProcessStartInfo]::new($pwsh)
    foreach ($argument in $Argumente) { $psi.ArgumentList.Add($argument) }
    $psi.WorkingDirectory = $Arbeitsordner
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.RedirectStandardInput = $true
    $psi.CreateNoWindow = $true
    $psi.StandardErrorEncoding = $oem
    foreach ($name in $Umgebung.Keys) {
        if ($null -eq $Umgebung[$name]) { [void]$psi.Environment.Remove($name) }
        else { $psi.Environment[$name] = [string]$Umgebung[$name] }
    }
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $prozess = [Diagnostics.Process]::Start($psi)
    $puffer = [IO.MemoryStream]::new()
    $kopie = $prozess.StandardOutput.BaseStream.CopyToAsync($puffer)
    $fehler = $prozess.StandardError.ReadToEndAsync()
    if ($Eingabe) {
        $bytes = $utf8.GetBytes($Eingabe)
        $prozess.StandardInput.BaseStream.Write($bytes, 0, $bytes.Length)
    }
    $prozess.StandardInput.Close()
    $frist = $false
    if (-not $prozess.WaitForExit($FristSekunden * 1000)) {
        $frist = $true
        $prozess.Kill($true)
        [void]$prozess.WaitForExit(5000)
    }
    [void]$kopie.Wait(5000)
    [void]$fehler.Wait(5000)
    $inhalt = $puffer.ToArray()
    $ergebnis = [pscustomobject]@{
        Exit = if ($frist) { 'Frist' } else { $prozess.ExitCode }
        Bytes = $inhalt
        Text = ($Kodierung.GetString($inhalt) -replace '\x1b\[[0-9;?]*[A-Za-z]', '')
        Fehler = ([string]$fehler.Result -replace '\x1b\[[0-9;?]*[A-Za-z]', '').Trim()
        Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 1)
        Sha = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($inhalt)).ToLowerInvariant()
    }
    $prozess.Dispose()
    return $ergebnis
}

function Schreibe-Probe {
    param([string]$Titel, [string]$Befehl, [object]$Ergebnis, [switch]$OhneText)
    Schreibe "--- $Titel"
    Schreibe "Befehl: $Befehl"
    Schreibe ('Exit {0} · {1} s · stdout {2} Bytes · sha256 {3}' -f $Ergebnis.Exit, $Ergebnis.Sekunden, $Ergebnis.Bytes.Length, $Ergebnis.Sha)
    if (-not $OhneText) { Schreibe $Ergebnis.Text.TrimEnd() }
    if ($Ergebnis.Fehler) { Schreibe "stderr: $($Ergebnis.Fehler)" }
    Schreibe ''
}

function Zeilen {
    param([string]$Text)
    return @($Text -split "`r?`n")
}

function Get-BasisBytes {
    param([string]$Pfad)
    $psi = [Diagnostics.ProcessStartInfo]::new('git')
    foreach ($argument in @('-C', $wurzel, 'show', "$($Basis):$Pfad")) { $psi.ArgumentList.Add($argument) }
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $prozess = [Diagnostics.Process]::Start($psi)
    $puffer = [IO.MemoryStream]::new()
    $kopie = $prozess.StandardOutput.BaseStream.CopyToAsync($puffer)
    $fehler = $prozess.StandardError.ReadToEndAsync()
    $prozess.WaitForExit()
    [void]$kopie.Wait()
    [void]$fehler.Wait()
    if ($prozess.ExitCode -ne 0) { throw "git show $($Basis):$Pfad scheiterte: $($fehler.Result)" }
    return , $puffer.ToArray()
}

function Setze-Klondatei {
    param([string]$Pfad, [ValidateSet('basis', 'neu')][string]$Fassung)
    $ziel = Join-Path $Klon $Pfad
    if ($Fassung -eq 'basis') { [IO.File]::WriteAllBytes($ziel, (Get-BasisBytes $Pfad)) }
    else { [IO.File]::WriteAllBytes($ziel, [IO.File]::ReadAllBytes((Join-Path $wurzel $Pfad))) }
}

function Get-PlanTabs {
    # Dieselbe Erkennung wie tools/dirigent/start-dirigent.ps1 (Get-PlanTabProcesses).
    return @(Get-CimInstance Win32_Process -Filter "Name = 'pwsh.exe'" | Where-Object {
        $_.CommandLine -match '(?i)^(?:"[^"]*"|\S+)\s+(?:-\S+\s+)*-File\s+(?:"[^"]*[\\/]plan-tab\.ps1"|[^"\s]*[\\/]plan-tab\.ps1)(?:\s|$)' -and
            $_.CommandLine -notmatch '(?i)\s-Einmal(?:\s|$)'
    })
}

function Get-ElternName {
    param([int]$ProzessId)
    $eltern = Get-CimInstance Win32_Process -Filter "ProcessId = $ProzessId" -ErrorAction SilentlyContinue
    if ($null -eq $eltern) { return '(beendet)' }
    return [string]$eltern.Name
}

function Schreibe-PlanTabs {
    param([string]$Zeitpunkt)
    $tabs = @(Get-PlanTabs)
    Schreibe "Plan-Tab-Prozesse $($Zeitpunkt): $($tabs.Count)"
    foreach ($tab in $tabs) {
        Schreibe ('  PID {0} · Elternprozess {1} ({2}) · {3}' -f $tab.ProcessId, (Get-ElternName $tab.ParentProcessId), $tab.ParentProcessId, $tab.CommandLine)
    }
    return , $tabs
}

function Warte-BisPlanTabsWeg {
    param([int]$Sekunden = 15)
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    while (@(Get-PlanTabs).Count -and $uhr.Elapsed.TotalSeconds -lt $Sekunden) { Start-Sleep -Milliseconds 300 }
    return [pscustomobject]@{ Anzahl = @(Get-PlanTabs).Count; Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 1) }
}

function Start-Platzhalter {
    return Start-Process $pwsh -ArgumentList '-NoProfile', '-Command', 'Start-Sleep -Seconds 300' -WindowStyle Hidden -PassThru
}

function Invoke-StarterSchleife {
    # Der Starter im Terminalmodus mit Ersatz-Claude: Runde 1 legt den Neustart-Marker,
    # Runde 2 endet ohne. Beobachtet werden die Plan-Tab-Prozesse ueber die ganze Laufzeit.
    param([string]$StarterPfad, [string[]]$Zusatz = @(), [string]$Arbeitsordner)
    $ordner = Join-Path $probe ('schleife-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
    [void][IO.Directory]::CreateDirectory($ordner)
    $psi = [Diagnostics.ProcessStartInfo]::new($pwsh)
    foreach ($argument in @('-NoLogo', '-NoProfile', '-File', $StarterPfad, '-InsideTerminal') + $Zusatz) { $psi.ArgumentList.Add($argument) }
    $psi.WorkingDirectory = $Arbeitsordner
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.RedirectStandardInput = $true
    $psi.CreateNoWindow = $true
    $psi.StandardOutputEncoding = $oem
    $psi.StandardErrorEncoding = $oem
    $psi.Environment['NAKAMA_DIRIGENT_CLAUDE'] = $ersatzClaude
    $psi.Environment['NAK256_PROBE_DIR'] = $ordner
    $psi.Environment['NAKAMA_DIRIGENT_ANKER'] = $ankerProbe
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $prozess = [Diagnostics.Process]::Start($psi)
    $prozess.StandardInput.Close()
    $ausgabe = $prozess.StandardOutput.ReadToEndAsync()
    $fehler = $prozess.StandardError.ReadToEndAsync()
    $zeitleiste = New-Object System.Collections.Generic.List[string]
    $pids = New-Object 'System.Collections.Generic.HashSet[int]'
    $hoechstens = 0
    $letzte = -1
    while (-not $prozess.HasExited -and $uhr.Elapsed.TotalSeconds -lt 120) {
        $tabs = @(Get-PlanTabs)
        foreach ($tab in $tabs) { [void]$pids.Add([int]$tab.ProcessId) }
        $hoechstens = [math]::Max($hoechstens, $tabs.Count)
        if ($tabs.Count -ne $letzte) {
            $beschreibung = ($tabs | ForEach-Object { "PID $($_.ProcessId) (gebunden an $([regex]::Match($_.CommandLine, '-BindenAn\s+(\d+)').Groups[1].Value))" }) -join ', '
            $zeitleiste.Add(('{0,5:0.0} s  Plan-Tab-Prozesse {1} {2}' -f $uhr.Elapsed.TotalSeconds, $tabs.Count, $beschreibung))
            $letzte = $tabs.Count
        }
        Start-Sleep -Milliseconds 300
    }
    if (-not $prozess.HasExited) { $prozess.Kill($true) }
    $ende = [math]::Round($uhr.Elapsed.TotalSeconds, 1)
    $zeitleiste.Add(('{0,5:0.0} s  Starter beendet, Exit {1}, Starter-PID {2}' -f $ende, $prozess.ExitCode, $prozess.Id))
    $weg = Warte-BisPlanTabsWeg
    $zeitleiste.Add(('{0,5:0.0} s  danach Plan-Tab-Prozesse {1}' -f ($ende + $weg.Sekunden), $weg.Anzahl))
    [void]$ausgabe.Wait(5000)
    [void]$fehler.Wait(5000)
    $zaehler = Join-Path $ordner 'runden.txt'
    return [pscustomobject]@{
        Exit = $prozess.ExitCode
        Zeitleiste = @($zeitleiste)
        Pids = @($pids)
        Hoechstens = $hoechstens
        NachEnde = $weg
        Runden = if ([IO.File]::Exists($zaehler)) { [int][IO.File]::ReadAllText($zaehler) } else { 0 }
        Text = ([string]$ausgabe.Result -replace '\x1b\[[0-9;?]*[A-Za-z]', '')
        Fehler = ([string]$fehler.Result).Trim()
    }
}

# ------------------------------------------------------------------ Kopf

$kopf = (& git -C $wurzel rev-parse HEAD).Trim()
Schreibe 'NAK-256 - Beine (a) bis (e), einzeln gefahren'
Schreibe ('Stand: {0}' -f (Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))
Schreibe "Basis-SHA: $Basis; HEAD beim Lauf: $kopf"
Schreibe "Skript: docs/beweise/roh/NAK-256-beine.ps1; Klon: $Klon; pwsh $($PSVersionTable.PSVersion); Windows Terminal $((Get-AppxPackage Microsoft.WindowsTerminal -ErrorAction SilentlyContinue | Select-Object -First 1).Version)"
Schreibe 'Gemessene Dateien (git hash-object):'
foreach ($pfad in 'tools/dirigent/cockpit.ps1', 'tools/dirigent/start-dirigent.ps1', 'tools/dirigent/plan-tab.ps1') {
    Schreibe ('  {0}  {1}' -f (& git -C $wurzel hash-object $pfad).Trim(), $pfad)
}
Schreibe 'Arbeitsbaum der Ticketpfade (git status --short):'
foreach ($zeile in @(& git -C $wurzel status --short -- tools/dirigent docs/beweise .claude/skills/dirigent)) { Schreibe "  $zeile" }
Schreibe ''

if (-not [IO.Directory]::Exists((Join-Path $Klon '.git'))) {
    & git clone --local -c core.longpaths=true $wurzel $Klon 2>&1 | Out-Null
    & git -C $Klon checkout --detach $Basis 2>&1 | Out-Null
}
$klonKopf = (& git -C $Klon rev-parse HEAD).Trim()
if ($klonKopf -ne $Basis) { throw "Klon $Klon steht auf $klonKopf, nicht auf $Basis" }
$klonCockpit = Join-Path $Klon 'tools\dirigent\cockpit.ps1'
$klonStarter = Join-Path $Klon 'tools\dirigent\start-dirigent.ps1'

# ------------------------------------------------------------------ Bein (b)

Schreibe '=== Bein (b) - cockpit.ps1 -Plan vor und nach dem Umbau bytegleich'
Schreibe "Ort: Klon am Basis-SHA. Vorher = tools/dirigent/cockpit.ps1 aus $Basis, nachher = dieselbe Datei aus dem Arbeitsbaum, beide an derselben Stelle."
Schreibe ''

function Vergleiche-Plan {
    param([string]$Name, [string[]]$PlanArgumente)
    $befehl = 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 ' + ($PlanArgumente -join ' ')
    Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'basis'
    $vorher = Invoke-Probe -Argumente (@('-NoProfile', '-File', $klonCockpit) + $PlanArgumente) -Arbeitsordner $Klon -Umgebung (Umgebung)
    Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'neu'
    $nachher = Invoke-Probe -Argumente (@('-NoProfile', '-File', $klonCockpit) + $PlanArgumente) -Arbeitsordner $Klon -Umgebung (Umgebung)
    Schreibe-Probe "$Name - vorher (Basis-SHA)" $befehl $vorher
    Schreibe-Probe "$Name - nachher (Arbeitsbaum); Text wie vorher, wenn bytegleich" $befehl $nachher -OhneText:($vorher.Sha -eq $nachher.Sha)
    Pruefe 'b' "$($Name): stdout und Exitcode bytegleich" (($vorher.Sha -eq $nachher.Sha) -and ($vorher.Exit -eq $nachher.Exit))
    return $nachher
}

Setze-Klondatei 'docs/PLAN-STAND.md' 'basis'
[void](Vergleiche-Plan 'b1 Planquellen nicht frisch (docs/PLAN-STAND.md des Basis-SHA)' @('-Plan'))
Push-Location $Klon
$planstand = & py -3.13 tools/plan/planstand.py 2>&1 | Out-String
Pop-Location
Schreibe "Klon: py -3.13 tools/plan/planstand.py -> $($planstand.Trim())"
Schreibe ''
$frisch = Vergleiche-Plan 'b2 frisch gerechneter Planstand' @('-Plan')
Pruefe 'b' 'b2: der frische Lauf zeigt den Gesamtplan (NAKAMA PLAN, Exit 0)' (($frisch.Exit -eq 0) -and $frisch.Text.StartsWith('NAKAMA PLAN · '))
[void](Vergleiche-Plan 'b3 mit Arbeitsanker' @('-Plan', '-CurrentStep', 'S26–28'))
[void](Vergleiche-Plan 'b4 unbekannter Arbeitsanker' @('-Plan', '-CurrentStep', 'S99'))

# ------------------------------------------------------------------ Bein (a)

Schreibe '=== Bein (a) - Statuszeile mit Beispiel-JSON'
Schreibe 'Eingabe: docs/beweise/roh/NAK-256-statuszeile-beispiel.json (Felder nach code.claude.com/docs/en/statusline; session_name nakama-dirigent, session_id nak256-probe)'
Schreibe ''
$json = [IO.File]::ReadAllText($beispiel)

Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'basis'
$rot = Invoke-Probe -Argumente @('-NoProfile', '-File', $klonCockpit, '-StatusLine') -Arbeitsordner $Klon -Umgebung (Umgebung) -Eingabe $json
Schreibe-Probe 'a0 Rotbeweis: Basis-SHA im Klon' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine < NAK-256-statuszeile-beispiel.json' $rot
$rotStand = @(Zeilen $rot.Text | Where-Object { $_ -match '^STAND\s' })
Pruefe 'a' 'a0 Rot: am Basis-SHA traegt die Statuszeile keine Zeile STAND' (($rot.Exit -eq 0) -and $rotStand.Count -eq 0)
Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'neu'
# Dieselbe Umgebung wie a0 (Klon: keine Worker, derselbe Git-Stand), damit nur der Umbau den Unterschied macht.
$a0n = Invoke-Probe -Argumente @('-NoProfile', '-File', $klonCockpit, '-StatusLine', '-Ticket', 'NAK-246') -Arbeitsordner $Klon -Umgebung (Umgebung) -Eingabe $json
Schreibe-Probe 'a0 Gegenstueck: Arbeitsbaum-Fassung im Klon, -Ticket NAK-246' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine -Ticket NAK-246 < NAK-256-statuszeile-beispiel.json' $a0n
$marken = { param($zeilen) @($zeilen | ForEach-Object { if ($_ -match '^(\S+)') { $Matches[1] } else { '' } }) }
$rotMarken = (& $marken (Zeilen $rot.Text)) -join '|'
$neuMarken = (& $marken (Zeilen $a0n.Text | Where-Object { $_ -notmatch '^STAND\s' })) -join '|'
Schreibe "Zeilenmarken vorher: $rotMarken"
Schreibe "Zeilenmarken nachher ohne STAND: $neuMarken"
Pruefe 'a' 'a0: alle bisherigen Zeilen bleiben in derselben Reihenfolge, STAND steht zwischen PLAN und FRAGE' (
    ($a0n.Exit -eq 0) -and ($rotMarken -eq $neuMarken) -and ((& $marken (Zeilen $a0n.Text)) -join '|') -match '\|PLAN\|STAND\|FRAGE\|')

$a1 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-StatusLine', '-Ticket', 'NAK-246') -Umgebung (Umgebung) -Eingabe $json
Schreibe-Probe 'a1 Arbeitsbaum, -Ticket NAK-246, ohne COLUMNS' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine -Ticket NAK-246 < NAK-256-statuszeile-beispiel.json' $a1
$a1Zeilen = Zeilen $a1.Text
$a1Stand = @($a1Zeilen | Where-Object { $_ -match '^STAND\s' })
Pruefe 'a' 'a1: genau eine Zeile STAND, Exit 0' (($a1.Exit -eq 0) -and $a1Stand.Count -eq 1)
Pruefe 'a' 'a1: STAND zeigt „läuft: NAK-246", „Etappe 4 von 6", „Wiederprüfung 1 steht aus"' (
    $a1Stand.Count -eq 1 -and $a1Stand[0].Contains('läuft: NAK-246 ') -and $a1Stand[0].Contains(' · Etappe 4 von 6 · ') -and $a1Stand[0].Contains(' · Wiederprüfung 1 steht aus · '))
Pruefe 'a' 'a1: STAND nennt die zwei naechsten offenen Planschritte' (
    $a1Stand.Count -eq 1 -and $a1Stand[0].EndsWith('danach: Die Klangregelung in der Sonde sicher und speicherbar machen → Dieselbe Klangregelung im Hauptprogramm bereitstellen'))

$a2 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-StatusLine', '-Ticket', 'NAK-246') -Umgebung (Umgebung @{ COLUMNS = '120' }) -Eingabe $json
Schreibe-Probe 'a2 Arbeitsbaum, -Ticket NAK-246, COLUMNS=120' 'COLUMNS=120 pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine -Ticket NAK-246 < NAK-256-statuszeile-beispiel.json' $a2
$a2Zeilen = Zeilen $a2.Text
$a2i = [array]::FindIndex([string[]]$a2Zeilen, [Predicate[string]] { param($x) $x -match '^STAND\s' })
Pruefe 'a' 'a2: bei 120 Spalten zwei Zeilen STAND und DANACH, jede kuerzer als 120 Zeichen' (
    $a2i -ge 0 -and $a2i + 1 -lt $a2Zeilen.Count -and $a2Zeilen[$a2i + 1] -match '^DANACH\s' -and $a2Zeilen[$a2i].Length -lt 120 -and $a2Zeilen[$a2i + 1].Length -lt 120)

$ank1 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Anker', '-Ticket', 'NAK-246') -Umgebung (Umgebung)
Schreibe-Probe 'a3 Ticketanker setzen' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Anker -Ticket NAK-246   (NAKAMA_DIRIGENT_ANKER = Proben-Anker)' $ank1
$ankerText = if ([IO.File]::Exists($ankerProbe)) { [IO.File]::ReadAllText($ankerProbe) } else { '' }
Schreibe "Proben-Anker: $ankerText"
Pruefe 'a' 'a3: -Anker -Ticket NAK-246 setzt den Anker (Exit 0)' (($ank1.Exit -eq 0) -and $ankerText.Contains('"Ticket":"NAK-246"'))
$a3 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-StatusLine') -Umgebung (Umgebung) -Eingabe $json
Schreibe-Probe 'a3 Statuszeile ohne -Ticket liest den Anker' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine < NAK-256-statuszeile-beispiel.json' $a3
$a3Stand = @(Zeilen $a3.Text | Where-Object { $_ -match '^STAND\s' })
Pruefe 'a' 'a3: STAND aus dem Anker gleicht a1' ($a3Stand.Count -eq 1 -and $a1Stand.Count -eq 1 -and $a3Stand[0] -eq $a1Stand[0])
$ank2 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Anker', '-Ticket', 'NAK-999') -Umgebung (Umgebung)
Schreibe-Probe 'a4 unbekanntes Ticket' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Anker -Ticket NAK-999' $ank2
Pruefe 'a' 'a4: ein unbekanntes Ticket wird abgewiesen (Exit 5), der Anker bleibt NAK-246' (($ank2.Exit -eq 5) -and [IO.File]::ReadAllText($ankerProbe).Contains('"Ticket":"NAK-246"'))
$ank3 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Anker') -Umgebung (Umgebung)
Schreibe-Probe 'a5 Ticketanker loesen' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Anker' $ank3
Pruefe 'a' 'a5: -Anker ohne Ticket loest den Anker (Exit 0, Datei weg)' (($ank3.Exit -eq 0) -and -not [IO.File]::Exists($ankerProbe))
$a5 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-StatusLine') -Umgebung (Umgebung) -Eingabe $json
Schreibe-Probe 'a5 Statuszeile ohne Anker' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -StatusLine < NAK-256-statuszeile-beispiel.json' $a5
$a5Stand = @(Zeilen $a5.Text | Where-Object { $_ -match '^STAND\s' })
Pruefe 'a' 'a5: ohne Anker folgt STAND „Als Nächstes" (SONDE-015) und behauptet kein laufendes Ticket' (
    $a5Stand.Count -eq 1 -and $a5Stand[0].Contains('als Nächstes: SONDE-015 Die Klangregelung in der Sonde sicher und speicherbar machen') -and -not $a5Stand[0].Contains('läuft:'))
Pruefe 'a' 'a5: die Kopfzeile von SONDE-015 folgt der Konvention nicht; STAND sagt „Schritt nicht lesbar" statt Pruefprosa mit SHA' (
    $a5Stand.Count -eq 1 -and $a5Stand[0].Contains(' · Etappe nicht lesbar · Schritt nicht lesbar · ') -and $a5Stand[0] -cnotmatch '\b[0-9a-f]{8}\b')
[IO.File]::WriteAllText($ankerProbe, '{kaputt')
$a6 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-StatusLine') -Umgebung (Umgebung) -Eingabe $json
[IO.File]::Delete($ankerProbe)
$a6Stand = @(Zeilen $a6.Text | Where-Object { $_ -match '^STAND\s' })
Schreibe "--- a6 beschaedigter Anker: $($a6Stand -join ' / ')"
Pruefe 'a' 'a6: ein beschaedigter Anker heisst „nicht lesbar", nie eine Erfindung' ($a6Stand.Count -eq 1 -and $a6Stand[0].Contains('nicht lesbar · Ticketanker nicht lesbar'))
$a7 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Uebersicht', '-Ticket', 'NAK-256') -Umgebung (Umgebung)
$a7Stand = [string]@(Zeilen $a7.Text)[0]
Schreibe '--- a7 Konvention am eigenen Manifest (docs/beweise/NAK-256.md, Kopfzeile „Etappe"): pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Uebersicht -Ticket NAK-256'
Schreibe $a7Stand
Pruefe 'a' 'a7: der Kopf dieses Manifests liest sich als „Etappe 1 von 1" mit dem laufenden Schritt' (
    ($a7.Exit -eq 0) -and $a7Stand.Contains('läuft: NAK-256 ') -and $a7Stand.Contains(' · Etappe 1 von 1 · Prüfung durch den Dirigenten steht aus · '))
Schreibe ''
foreach ($datei in @(Get-ChildItem -LiteralPath ([IO.Path]::GetTempPath()) -Filter 'nakama-dirigent-nak256-probe-*' -File)) {
    [IO.File]::Delete($datei.FullName)
    Schreibe "Aufgeraeumt: $($datei.Name)"
}
Schreibe ''

# ------------------------------------------------------------------ Bein (c)

Schreibe '=== Bein (c) - plan-tab.ps1 -Einmal'
$c0 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Plan') -Umgebung (Umgebung)
Schreibe-Probe 'c0 Vergleich: -Plan im Arbeitsbaum zur selben Zeit' 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan' $c0
$c1 = Invoke-Probe -Argumente @('-NoProfile', '-File', $planTab, '-Einmal') -Umgebung (Umgebung)
Schreibe-Probe 'c1 plan-tab.ps1 -Einmal im Arbeitsbaum' 'pwsh -NoProfile -File tools/dirigent/plan-tab.ps1 -Einmal' $c1
$c1Zeilen = @(Zeilen $c1.Text.TrimEnd())
Pruefe 'c' 'c1: Exit 0' ($c1.Exit -eq 0)
Pruefe 'c' 'c1: oben STAND, darunter der Gesamtplan mit Phasen und Schritten, unten „aktualisiert HH:mm:ss"' (
    $c1Zeilen.Count -gt 10 -and $c1Zeilen[0] -match '^STAND\s' -and @($c1Zeilen | Where-Object { $_ -match '^NAKAMA PLAN · \d+ / \d+ fertig · \d+ offen$' }).Count -eq 1 -and
    @($c1Zeilen | Where-Object { $_ -match '^[✓◐○→] ' }).Count -ge 40 -and $c1Zeilen[-1] -match '^aktualisiert \d{2}:\d{2}:\d{2} · ')
if ($c0.Exit -eq 4) {
    Pruefe 'c' 'c1: auch wenn -Plan „nicht frisch" meldet (c0 Exit 4), zeigt der Tab die letzte Planrechnung mit Hinweis' (
        @($c1Zeilen | Where-Object { $_ -match '^Planrechnung vom \d{2}\.\d{2}\.\d{4} · neuere Belege und Planänderungen sind noch nicht eingerechnet$' }).Count -eq 1)
}
$c2 = Invoke-Probe -Argumente @('-NoProfile', '-File', $cockpit, '-Uebersicht', '-Hoehe', '30') -Umgebung (Umgebung @{ COLUMNS = '120' })
Schreibe-Probe 'c2 Uebersicht fuer ein Fenster mit 30 Zeilen und 120 Spalten' 'COLUMNS=120 pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Uebersicht -Hoehe 30' $c2
$c2Zeilen = @(Zeilen $c2.Text.TrimEnd())
Pruefe 'c' 'c2: hoechstens 30 Zeilen, jede kuerzer als 120 Zeichen, fertige Phasen gefaltet' (
    $c2Zeilen.Count -le 30 -and @($c2Zeilen | Where-Object { $_.Length -ge 120 }).Count -eq 0 -and @($c2Zeilen | Where-Object { $_ -match '^✓ .+ · \d+ von \d+ fertig$' }).Count -ge 1)
& git -C $wurzel cat-file -e "$($Basis):tools/dirigent/plan-tab.ps1" 2>$null
Pruefe 'c' 'Rot: am Basis-SHA gibt es tools/dirigent/plan-tab.ps1 nicht' ($LASTEXITCODE -ne 0)
Schreibe ''

# ------------------------------------------------------------------ Bein (d)

Schreibe '=== Bein (d) - Starter oeffnet genau einen Plan-Tab'
$vorab = @(Get-PlanTabs)
Pruefe 'd' 'Voraussetzung: vor dem Bein laeuft kein Plan-Tab' ($vorab.Count -eq 0)
Pruefe 'd' 'Voraussetzung: kein Neustart-Marker liegt (sonst wuerde die Probe einen echten Neustart verschlucken)' (-not [IO.File]::Exists($marker))
if ($vorab.Count -eq 0 -and -not [IO.File]::Exists($marker)) {
    Setze-Klondatei 'tools/dirigent/start-dirigent.ps1' 'basis'
    $d0 = Invoke-Probe -Argumente @('-NoProfile', '-File', $klonStarter, '-NurPlanTab') -Arbeitsordner $Klon -Umgebung (Umgebung)
    Schreibe-Probe 'd0 Rotbeweis: Starter des Basis-SHA' 'pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1 -NurPlanTab' $d0
    Pruefe 'd' 'd0 Rot: der Starter des Basis-SHA kennt -NurPlanTab nicht' (($d0.Exit -ne 0) -and $d0.Fehler.Contains('NurPlanTab'))

    $rotSchleife = Invoke-StarterSchleife -StarterPfad $klonStarter -Arbeitsordner $Klon
    Schreibe '--- d0 Rotbeweis: Starter des Basis-SHA im Terminalmodus, Ersatz-Claude mit Marker-Neustart'
    Schreibe "Befehl: NAKAMA_DIRIGENT_CLAUDE=docs/beweise/roh/NAK-256-ersatz-claude.ps1 pwsh -NoLogo -NoProfile -File tools/dirigent/start-dirigent.ps1 -InsideTerminal"
    foreach ($zeile in $rotSchleife.Zeitleiste) { Schreibe $zeile }
    Schreibe "Runden des Ersatz-Claude: $($rotSchleife.Runden)"
    Schreibe ''
    Pruefe 'd' 'd0 Rot: am Basis-SHA oeffnet der Starter ueber zwei Runden keinen Plan-Tab' (($rotSchleife.Runden -eq 2) -and $rotSchleife.Pids.Count -eq 0)

    $platz = Start-Platzhalter
    $d1Befehl = @('-NoProfile', '-File', $starter, '-NurPlanTab', '-PlanTabFenster', 'nak256-probe', '-PlanTabBindung', [string]$platz.Id)
    $d1a = Invoke-Probe -Argumente $d1Befehl -Umgebung (Umgebung)
    Schreibe-Probe 'd1 Trockenlauf mit Windows Terminal, erster Aufruf' "pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1 -NurPlanTab -PlanTabFenster nak256-probe -PlanTabBindung $($platz.Id)   (Platzhalterprozess)" $d1a
    $d1Tabs = Schreibe-PlanTabs 'nach dem ersten Aufruf'
    $d1b = Invoke-Probe -Argumente $d1Befehl -Umgebung (Umgebung)
    Schreibe-Probe 'd1 Trockenlauf mit Windows Terminal, zweiter Aufruf' 'derselbe Befehl' $d1b
    $d1Tabs2 = Schreibe-PlanTabs 'nach dem zweiten Aufruf'
    Pruefe 'd' 'd1: der erste Aufruf oeffnet den Plan-Tab als Tab in Windows Terminal' (
        ($d1a.Exit -eq 0) -and $d1a.Text.Contains('PLAN-TAB · geöffnet · PID') -and $d1a.Text.Contains('· Windows Terminal ·') -and
        $d1Tabs.Count -eq 1 -and (Get-ElternName $d1Tabs[0].ParentProcessId) -eq 'WindowsTerminal.exe' -and $d1Tabs[0].CommandLine.Contains("-BindenAn $($platz.Id)"))
    Pruefe 'd' 'd1: der zweite Aufruf oeffnet keinen zweiten Tab' (
        ($d1b.Exit -eq 0) -and $d1b.Text.Contains('PLAN-TAB · läuft schon') -and $d1Tabs2.Count -eq 1 -and $d1Tabs2[0].ProcessId -eq $d1Tabs[0].ProcessId)
    $platz.Kill()
    $d1Weg = Warte-BisPlanTabsWeg
    Schreibe "Platzhalter beendet; Plan-Tab-Prozesse nach $($d1Weg.Sekunden) s: $($d1Weg.Anzahl)"
    Pruefe 'd' 'd1: endet der gebundene Prozess, endet der Plan-Tab von selbst' ($d1Weg.Anzahl -eq 0)
    Schreibe ''

    $ohneWt = Join-Path $probe 'ohne-windows-terminal'
    [void][IO.Directory]::CreateDirectory($ohneWt)
    $platz = Start-Platzhalter
    $d2Befehl = @('-NoProfile', '-File', $starter, '-NurPlanTab', '-PlanTabBindung', [string]$platz.Id)
    $d2a = Invoke-Probe -Argumente $d2Befehl -Umgebung (Umgebung @{ LOCALAPPDATA = $ohneWt })
    Schreibe-Probe 'd2 ohne Windows Terminal (LOCALAPPDATA ohne Terminal-Einstellungen), erster Aufruf' "LOCALAPPDATA=<leer> pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1 -NurPlanTab -PlanTabBindung $($platz.Id)" $d2a
    $d2Tabs = Schreibe-PlanTabs 'nach dem ersten Aufruf'
    $d2b = Invoke-Probe -Argumente $d2Befehl -Umgebung (Umgebung @{ LOCALAPPDATA = $ohneWt })
    Schreibe-Probe 'd2 ohne Windows Terminal, zweiter Aufruf' 'derselbe Befehl' $d2b
    $d2Tabs2 = Schreibe-PlanTabs 'nach dem zweiten Aufruf'
    Pruefe 'd' 'd2: ohne Windows Terminal oeffnet der Starter ein eigenes Fenster' (
        ($d2a.Exit -eq 0) -and $d2a.Text.Contains('PLAN-TAB · geöffnet · PID') -and $d2a.Text.Contains('· eigenes Fenster ·') -and
        $d2Tabs.Count -eq 1 -and (Get-ElternName $d2Tabs[0].ParentProcessId) -ne 'WindowsTerminal.exe')
    Pruefe 'd' 'd2: der zweite Aufruf oeffnet kein zweites Fenster' (
        ($d2b.Exit -eq 0) -and $d2b.Text.Contains('PLAN-TAB · läuft schon') -and $d2Tabs2.Count -eq 1 -and $d2Tabs2[0].ProcessId -eq $d2Tabs[0].ProcessId)
    $platz.Kill()
    $d2Weg = Warte-BisPlanTabsWeg
    Schreibe "Platzhalter beendet; Plan-Tab-Prozesse nach $($d2Weg.Sekunden) s: $($d2Weg.Anzahl)"
    Pruefe 'd' 'd2: endet der gebundene Prozess, schliesst das Fenster' ($d2Weg.Anzahl -eq 0)
    Schreibe ''

    $schleife = Invoke-StarterSchleife -StarterPfad $starter -Zusatz @('-PlanTabFenster', 'nak256-probe') -Arbeitsordner $wurzel
    Schreibe '--- d3 Starter im Terminalmodus, Ersatz-Claude mit Marker-Neustart'
    Schreibe "Befehl: NAKAMA_DIRIGENT_CLAUDE=docs/beweise/roh/NAK-256-ersatz-claude.ps1 pwsh -NoLogo -NoProfile -File tools/dirigent/start-dirigent.ps1 -InsideTerminal -PlanTabFenster nak256-probe"
    foreach ($zeile in $schleife.Zeitleiste) { Schreibe $zeile }
    Schreibe "Runden des Ersatz-Claude: $($schleife.Runden); verschiedene Plan-Tab-PIDs: $($schleife.Pids.Count); hoechstens gleichzeitig: $($schleife.Hoechstens)"
    Schreibe 'Ausgabe des Starters (ohne Logo):'
    foreach ($zeile in @(Zeilen $schleife.Text | Where-Object { $_ -match 'Gesamtplan|ERSATZ-CLAUDE|Neustart|Dirigent beendet|Exitcode|Enter =' })) { Schreibe "  $zeile" }
    if ($schleife.Fehler) { Schreibe "stderr: $($schleife.Fehler)" }
    Schreibe ''
    # Der Starter schreibt im Terminalmodus mit der Konsolen-Codepage; die Umlaute sind deshalb offen gelassen.
    $geoeffnet = @(Zeilen $schleife.Text | Where-Object { $_ -match '^Gesamtplan im zweiten Tab ge.{1,2}ffnet\.$' }).Count
    $laeuft = @(Zeilen $schleife.Text | Where-Object { $_ -match '^Gesamtplan l.{1,2}uft bereits\.$' }).Count
    Pruefe 'd' 'd3: Start oeffnet den Plan-Tab, der Marker-Neustart oeffnet keinen zweiten' (
        ($schleife.Runden -eq 2) -and $schleife.Pids.Count -eq 1 -and $schleife.Hoechstens -eq 1 -and $geoeffnet -eq 1 -and $laeuft -eq 1)
    Pruefe 'd' 'd3: endet das Dirigentenfenster (Starter), endet der Plan-Tab' ($schleife.NachEnde.Anzahl -eq 0)
    Pruefe 'd' 'd3: die Probe hinterlaesst keinen Neustart-Marker' (-not [IO.File]::Exists($marker))
    Setze-Klondatei 'tools/dirigent/start-dirigent.ps1' 'neu'
}
Schreibe ''

# ------------------------------------------------------------------ Bein (e)

Schreibe '=== Bein (e) - Beobachter mit Fable-Werten'
$eArgumente = @('-WatchWorker', '-WorkerId', 'nak256-nicht-da', '-BaseSha', $Basis, '-Aufsicht', 'NORMAL', '-StartModel', 'Fable', '-StartEffort', 'xhigh', '-DirigentSessionId', 'nak256-probe', '-TimeoutSeconds', '60')
Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'basis'
$e0 = Invoke-Probe -Argumente (@('-NoProfile', '-File', $klonCockpit) + $eArgumente) -Arbeitsordner $Klon -Umgebung (Umgebung)
Setze-Klondatei 'tools/dirigent/cockpit.ps1' 'neu'
$eBefehl = 'pwsh -NoProfile -File tools/dirigent/cockpit.ps1 ' + ($eArgumente -join ' ')
Schreibe-Probe 'e0 Rotbeweis: Basis-SHA im Klon' $eBefehl $e0
Pruefe 'e' 'e0 Rot: am Basis-SHA scheitert -StartModel Fable an der Parameterpruefung' (($e0.Exit -ne 0) -and $e0.Fehler.Contains('StartModel'))
$e1 = Invoke-Probe -Argumente (@('-NoProfile', '-File', $cockpit) + $eArgumente) -Umgebung (Umgebung)
Schreibe-Probe 'e1 Arbeitsbaum, Worker-ID existiert nicht' $eBefehl $e1
Pruefe 'e' 'e1: der Beobachter startet ohne Validierungsfehler und meldet als erstes den fehlenden Worker (Exit 6)' (
    ($e1.Exit -eq 6) -and -not $e1.Fehler -and @(Zeilen $e1.Text.TrimEnd())[0] -eq 'WORKER nak256-nicht-da · unerwartet verschwunden')
$e2 = Invoke-Probe -Argumente @('-NoProfile', '-Command', "& '$cockpit' -WatchWorker -WorkerId nak256-nicht-da -BaseSha $Basis -StartModel ('Fable' + [char]10 + 'x') -DirigentSessionId nak256-probe") -Umgebung (Umgebung)
Schreibe-Probe 'e2 Zeilenumbruch im Modellwert' "pwsh -NoProfile -Command ""& 'tools/dirigent/cockpit.ps1' -WatchWorker ... -StartModel ('Fable' + [char]10 + 'x') ...""" $e2
Pruefe 'e' 'e2: ein Zeilenumbruch im Anzeigewert bleibt gesperrt (eine Ausgabezeile = ein Monitor-Ereignis)' (($e2.Exit -ne 0) -and $e2.Fehler.Contains('StartModel'))
Schreibe ''

# ------------------------------------------------------------------ Bilanz

$erfuellt = @($bilanz | Where-Object { $_.Erfuellt }).Count
Schreibe "=== BILANZ: $erfuellt von $($bilanz.Count) Pruefungen erfuellt"
foreach ($eintrag in $bilanz) {
    Schreibe ('  ({0}) {1} - {2}' -f $eintrag.Bein, $(if ($eintrag.Erfuellt) { 'ja  ' } else { 'NEIN' }), $eintrag.Zusage)
}
[IO.File]::WriteAllText($roh, (($aus -join "`n").TrimEnd() + "`n"), $utf8)
try { [IO.Directory]::Delete($probe, $true) } catch { }
Write-Output "BILANZ: $erfuellt von $($bilanz.Count) Pruefungen erfuellt · Rohausgabe $roh"
if ($erfuellt -ne $bilanz.Count) { exit 1 }
exit 0
