<#
.SYNOPSIS
    Laufzeit-Arm (Plan S25e, Register NAK-286): misst den gebauten Stand im
    echten FL Studio - nach gruenem Kanon, automatisch, ohne Nachfrage.

.DESCRIPTION
    Ablauf:
      1. Lohnt es?  Diff Basis..HEAD gegen die Produktpfade (Plugin, Schemas,
         Broker, Installer). Ohne Treffer: begruendet uebersprungen, Exit 0.
      2. Installation (ausser -OhneInstallation): Manifest-Hashes nachziehen
         (pruefe_installer_manifest.py --hashen), Diagnose-FL beenden, erhoehte
         Aufgabe \Nakama\installieren (NAK-285) starten und auf ihr Ergebnis
         warten, danach \Nakama\pruefen fuer das Urteil "aktuell".
         Ein FL mit fremdem Projekt im Fenster heisst: uebersprungen, Exit 0 -
         ein fremdes FL wird nie beendet.
      3. Bereitschaft: loopMIDI VOR FL (FL sieht nur MIDI-Ports, die beim
         Start existieren), fensterlose FL-Restprozesse beenden, Projektordner
         unter %LOCALAPPDATA%\evenacadia\nakama-laufzeit\projekt aus dem Repo
         befuellen (Diagnoseprojekt plus Samples\Testtrack.wav), FL mit dem
         Projekt starten, auf den Ping des Controller-Skripts warten.
      4. Szenarien: jede JSON-Datei unter docs/gesundheit/szenarien/ ueber
         tools/fl/szenario.py, Rohausgabe in EINE Markdown-Datei.
      5. Kopfzeile "LAUFZEIT <Ticket> <sha> <Urteil> ..." auf stdout - sie
         gehoert ins Manifest.

    Exit 0 = gemessen oder begruendet uebersprungen, 3 = Voraussetzung fehlt
    (Hashes, Installation, Port, Ping), 4 = mindestens ein Szenario verfehlt.
    Das Diagnoseprojekt wird nie gespeichert; FL bleibt danach offen
    (-Beenden schliesst es, z. B. im naechtlichen Lauf).

.EXAMPLE
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket NAK-286 -Basis 467e4534
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket PROBE -Erzwingen -OhneInstallation -Neustart
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Ticket,
    [string]$Basis,
    [switch]$Erzwingen,
    [switch]$OhneInstallation,
    [switch]$Neustart,
    [switch]$Beenden,
    [string]$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [string]$McpRepo = 'C:\Users\phili\Projekte\fl-studio-mcp',
    [string]$Projekt,
    [string]$Sample,
    [string]$Roh,
    [string[]]$Szenarien,
    [string]$LoopMidi = 'C:\Program Files (x86)\Tobias Erichsen\loopMIDI\loopMIDI.exe',
    [int]$PingFristSekunden = 150,
    [int]$InstallFristSekunden = 900
)

$ErrorActionPreference = 'Stop'
$env:PYTHONIOENCODING = 'utf-8'

$Produktpfade = @('eq-copilot/plugin/', 'eq-copilot/schemas/', 'broker/src/', 'eq-copilot/install/')
$arbeit = Join-Path $env:LOCALAPPDATA 'evenacadia\nakama-laufzeit'
$projektOrdner = Join-Path $arbeit 'projekt'
$logDatei = Join-Path $arbeit 'laufzeit.log'
New-Item -ItemType Directory -Force -Path $projektOrdner | Out-Null

if (-not $Projekt) { $Projekt = Join-Path $Repo 'eq-copilot\fixtures\fl\Nakama-Diagnose.flp' }
$projektName = [IO.Path]::GetFileName($Projekt)
$szenarioPy = Join-Path $Repo 'tools\fl\szenario.py'
$head = (& git -C $Repo rev-parse --short=8 HEAD 2>$null)
if (-not $head) { $head = 'ohne-git' }
if (-not $Roh) { $Roh = Join-Path $Repo ("docs\beweise\roh\{0}-laufzeit-{1}.md" -f $Ticket, $head) }

$script:protokoll = New-Object System.Collections.Generic.List[string]
function Zeit { (Get-Date).ToString('yyyy-MM-dd HH:mm:ss') }
function Log([string]$text) {
    $zeile = "[{0}] {1}" -f (Zeit), $text
    Write-Host $zeile
    Add-Content -LiteralPath $logDatei -Value $zeile -Encoding utf8
    $script:protokoll.Add($zeile)
}
function Schreibe-Roh([string]$urteil, [string]$zusatz) {
    $kopf = @(
        "# Laufzeit-Arm $Ticket - $urteil",
        "",
        "Zeit: $(Zeit) | Basis: $Basis | HEAD: $head | Repo: $Repo | Projekt: $projektName",
        "Kopfzeile: LAUFZEIT $Ticket $head $urteil $zusatz",
        "",
        "## Protokoll",
        "",
        '```text'
    ) + @($script:protokoll) + @('```', '')
    New-Item -ItemType Directory -Force -Path (Split-Path $Roh) | Out-Null
    $szenarienTeil = ''
    if (Test-Path -LiteralPath $Roh) {
        $alt = Get-Content -LiteralPath $Roh -Raw -Encoding utf8
        if ($alt) {
            $idx = ([string]$alt).IndexOf('## Szenarien')
            if ($idx -ge 0) { $szenarienTeil = ([string]$alt).Substring($idx) }
        }
    }
    Set-Content -LiteralPath $Roh -Value (($kopf -join "`n") + $szenarienTeil) -Encoding utf8
}
function Ende([int]$code, [string]$urteil, [string]$zusatz = '') {
    Schreibe-Roh $urteil $zusatz
    Write-Host ("LAUFZEIT {0} {1} {2} {3} roh={4}" -f $Ticket, $head, $urteil, $zusatz, $Roh)
    exit $code
}

# ---------------------------------------------------------------- Hilfen
function FL-Instanzen {
    Get-Process FL64 -ErrorAction SilentlyContinue | ForEach-Object {
        [pscustomobject]@{ Id = $_.Id; Titel = $_.MainWindowTitle; Fenster = ([int64]$_.MainWindowHandle -ne 0) }
    }
}
function Ist-Diagnose($inst) { $inst.Fenster -and $inst.Titel -like "*$projektName*" }
function Ist-Fremd($inst)    { $inst.Fenster -and -not ($inst.Titel -like "*$projektName*") }

function Beende-Diagnose-FL {
    foreach ($i in (FL-Instanzen | Where-Object { Ist-Diagnose $_ })) {
        Log "Diagnose-FL beenden: PID $($i.Id) '$($i.Titel)' (Projekt wird nie gespeichert)"
        Stop-Process -Id $i.Id -Force -ErrorAction SilentlyContinue
    }
    $t0 = Get-Date
    while ((FL-Instanzen | Where-Object { Ist-Diagnose $_ }) -and ((Get-Date) - $t0).TotalSeconds -lt 30) { Start-Sleep 1 }
}
function Beende-Restprozesse {
    foreach ($i in (FL-Instanzen | Where-Object { -not $_.Fenster })) {
        Stop-Process -Id $i.Id -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 500
        # Get-Process -Id schweigt bei Zugriffsverweigerung; die Liste lügt nicht.
        $weg = -not (FL-Instanzen | Where-Object { $_.Id -eq $i.Id })
        Log ("FL-Restprozess PID {0}: {1}" -f $i.Id, $(if ($weg) { 'beendet' } else { 'nicht beendbar (Zugriff), ignoriert' }))
    }
}

function Boot-Marke {
    # Vom Controller-Skript in OnInit geschrieben; ohne MIDI-Pfad lesbar.
    $marke = Boot-Marken-Pfad
    if (-not (Test-Path -LiteralPath $marke)) { return "keine Boot-Marke ($marke)" }
    $m = Get-Item -LiteralPath $marke
    return ("Boot-Marke {0} ({1})" -f $m.LastWriteTime.ToString('HH:mm:ss'), ((Get-Content -LiteralPath $marke -Raw) -replace '\s+', ' '))
}

function Lohnt-Es {
    if ($Erzwingen) { return @{ Ja = $true; Grund = 'erzwungen (-Erzwingen)' } }
    if (-not $Basis) { return @{ Ja = $true; Grund = 'ohne Basis-SHA: immer' } }
    $dateien = @(& git -C $Repo diff --name-only "$Basis..HEAD" 2>&1)
    if ($LASTEXITCODE -ne 0) { return @{ Ja = $true; Grund = "Diff nicht bestimmbar ($($dateien -join ' ')): vorsichtshalber ja" } }
    $treffer = @($dateien | Where-Object { $d = $_; $Produktpfade | Where-Object { $d.StartsWith($_) } })
    if ($treffer.Count -gt 0) { return @{ Ja = $true; Grund = "Produktpfade im Diff: $($treffer.Count) Datei(en), z. B. $($treffer[0])" } }
    return @{ Ja = $false; Grund = "Diff $Basis..HEAD ($($dateien.Count) Datei(en)) beruehrt keinen Produktpfad" }
}

function Aufgabe([string]$name) {
    $t = Get-ScheduledTask -TaskPath '\Nakama\' -TaskName $name -ErrorAction SilentlyContinue
    if (-not $t) { throw "Aufgabe \Nakama\$name fehlt - einmal erhoeht tools/dirigent/install-aufgaben-registrieren.ps1 ausfuehren (NAK-285)" }
    Start-ScheduledTask -TaskPath '\Nakama\' -TaskName $name
    $t0 = Get-Date
    do {
        Start-Sleep 1
        $state = (Get-ScheduledTask -TaskPath '\Nakama\' -TaskName $name).State
    } while ($state -eq 'Running' -and ((Get-Date) - $t0).TotalSeconds -lt $InstallFristSekunden)
    $info = Get-ScheduledTaskInfo -TaskPath '\Nakama\' -TaskName $name
    $log = Join-Path $env:LOCALAPPDATA "evenacadia\nakama-installer\$name.log"
    $tail = if (Test-Path -LiteralPath $log) { (Get-Content -LiteralPath $log -Tail 4) -join ' | ' } else { '(kein Log)' }
    return @{ Exit = $info.LastTaskResult; Zustand = $state; Log = $tail }
}

function Installiere {
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { return @{ Ok = $false; Ueberspringen = $true; Grund = "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" } }
    Beende-Diagnose-FL
    Beende-Restprozesse
    Log 'Manifest-Hashes nachziehen (--hashen)'
    & py -3.13 (Join-Path $Repo 'tools\eq-copilot\pruefe_installer_manifest.py') --hashen 2>&1 | ForEach-Object { Log "  hashen: $_" }
    if ($LASTEXITCODE -ne 0) { return @{ Ok = $false; Ueberspringen = $false; Grund = "--hashen Exit $LASTEXITCODE" } }
    Log 'Aufgabe \Nakama\installieren starten'
    $r = Aufgabe 'installieren'
    Log ("  installieren: Exit {0}, Zustand {1}, Log: {2}" -f $r.Exit, $r.Zustand, $r.Log)
    if ($r.Exit -ne 0 -or $r.Zustand -eq 'Running') { return @{ Ok = $false; Ueberspringen = $false; Grund = "Installation Exit $($r.Exit) ($($r.Zustand))" } }
    $p = Aufgabe 'pruefen'
    Log ("  pruefen: Exit {0}, Log: {1}" -f $p.Exit, $p.Log)
    return @{ Ok = $true; Ueberspringen = $false; Grund = "installiert; pruefen Exit $($p.Exit)" }
}

function Stelle-LoopMidi-Sicher {
    if (Get-Process loopMIDI -ErrorAction SilentlyContinue) { Log 'loopMIDI laeuft'; return }
    if (-not (Test-Path -LiteralPath $LoopMidi)) { throw "loopMIDI fehlt unter $LoopMidi" }
    Start-Process -FilePath $LoopMidi | Out-Null
    Start-Sleep 3
    Log 'loopMIDI gestartet (Port aus der Registry wird beim Start angelegt)'
}

function Bereite-Projekt {
    if (-not (Test-Path -LiteralPath $Projekt)) { throw "Diagnoseprojekt fehlt: $Projekt" }
    Copy-Item -LiteralPath $Projekt -Destination (Join-Path $projektOrdner $projektName) -Force
    $samples = Join-Path $projektOrdner 'Samples'
    New-Item -ItemType Directory -Force -Path $samples | Out-Null
    $ziel = Join-Path $samples 'Testtrack.wav'
    if (-not (Test-Path -LiteralPath $ziel)) {
        # Testtrack.wav ist ein Maschinenartefakt (nicht im Git); Kandidaten
        # in dieser Reihenfolge, -Sample ueberschreibt.
        $kandidaten = @($Sample) + @(
            (Join-Path $Repo 'eq-copilot\kalibration\Testtrack.wav'),
            'C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav',
            (Join-Path $env:USERPROFILE 'OneDrive\Desktop\Skripte\Project_1\Samples\Testtrack.wav')
        ) | Where-Object { $_ -and (Test-Path -LiteralPath $_) }
        if ($kandidaten.Count -eq 0) { throw "Testtrack.wav fehlt (kein Kandidat vorhanden); -Sample <pfad> angeben oder nach $samples legen" }
        Copy-Item -LiteralPath $kandidaten[0] -Destination $ziel -Force
        Log "Testtrack.wav aus $($kandidaten[0]) nach $samples kopiert"
    }
    return (Join-Path $projektOrdner $projektName)
}

function FL-Pfad {
    try { $p = (Get-ItemProperty 'HKCU:\Software\Image-Line\Shared\Paths' -ErrorAction Stop).'FL Studio'; if ($p -and (Test-Path -LiteralPath $p)) { return $p } } catch { }
    return 'C:\Program Files\Image-Line\FL Studio 2026\FL64.exe'
}

function Starte-FL([string]$flp) {
    $exe = FL-Pfad
    $p = Start-Process -FilePath $exe -ArgumentList ('"' + $flp + '"') -PassThru
    Log "FL gestartet: PID $($p.Id) ($exe) mit $flp"
    $script:flStart = Get-Date
    return $p.Id
}

function Boot-Marken-Pfad {
    $basis = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders' -ErrorAction SilentlyContinue).Personal
    if ($basis) { $basis = [Environment]::ExpandEnvironmentVariables($basis) } else { $basis = Join-Path $env:USERPROFILE 'Documents' }
    return (Join-Path $basis 'Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\mcp_boot.json')
}

function Warte-Boot {
    # Gemessen 12.09.2026: ein Trigger, der FL waehrend der Initialisierung
    # trifft, laesst die Instanz danach dauerhaft stumm. Deshalb erst auf die
    # Boot-Marke des Controller-Skripts (OnInit) warten, dann eine Schonfrist
    # fuer das Laden des Projekts - und vorher KEINEN einzigen Trigger senden.
    $marke = Boot-Marken-Pfad
    $t0 = Get-Date
    while (((Get-Date) - $t0).TotalSeconds -lt 60) {
        if ((Test-Path -LiteralPath $marke) -and ((Get-Item -LiteralPath $marke).LastWriteTime -ge $script:flStart)) {
            Log ("Boot-Marke nach {0} s: {1}" -f [int]((Get-Date) - $t0).TotalSeconds, ((Get-Content -LiteralPath $marke -Raw) -replace '\s+', ' '))
            Start-Sleep 8
            return
        }
        Start-Sleep 1
    }
    Log 'keine Boot-Marke binnen 60 s (altes Controller-Skript?); Schonfrist 30 s'
    Start-Sleep 30
}

function Ping-FL {
    $ausgabe = & uv run --directory $McpRepo --python 3.12 python $szenarioPy --ping 2>&1
    $zeile = @($ausgabe | Where-Object { $_ -like '{*' }) | Select-Object -Last 1
    if (-not $zeile) { return @{ Ok = $false; Fehler = ($ausgabe -join ' ').Trim() } }
    try { $j = $zeile | ConvertFrom-Json } catch { return @{ Ok = $false; Fehler = $zeile } }
    if (-not $j.success) { return @{ Ok = $false; Fehler = $j.error } }
    return @{ Ok = $true; Titel = $j.program_title; Version = $j.fl_version; Port = $j.port_name }
}

function Warte-Ping {
    # ui.getProgTitle() liefert nur "FL Studio 2026" (gemessen 12.09.2026);
    # welches Projekt offen ist, sagt der Fenstertitel der Instanz (FL-Instanzen).
    $t0 = Get-Date
    $letzter = ''
    while (((Get-Date) - $t0).TotalSeconds -lt $PingFristSekunden) {
        $r = Ping-FL
        if ($r.Ok) {
            $inst = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
            if ($inst.Count -gt 0) { Log ("Ping ok: FL {0} ('{1}'), Fenster '{2}', Port {3}" -f $r.Version, $r.Titel, $inst[0].Titel, $r.Port); return $r }
            $letzter = "Ping antwortet, aber kein Fenster mit $projektName"
        } else { $letzter = $r.Fehler }
        Start-Sleep 5
    }
    throw "kein Ping binnen $PingFristSekunden s: $letzter; $(Boot-Marke)"
}

function Fahre-Szenarien {
    if (-not $Szenarien) { $Szenarien = @(Get-ChildItem (Join-Path $Repo 'docs\gesundheit\szenarien\*.json') | Sort-Object Name | ForEach-Object { $_.FullName }) }
    if ($Szenarien.Count -eq 0) { throw 'keine Szenarien gefunden' }
    Add-Content -LiteralPath $Roh -Value "`n## Szenarien`n" -Encoding utf8
    $ergebnisse = @()
    foreach ($s in $Szenarien) {
        $ausgabe = & uv run --directory $McpRepo --python 3.12 python $szenarioPy $s --roh $Roh 2>&1
        $code = $LASTEXITCODE
        $ergebnisse += [pscustomobject]@{ Datei = [IO.Path]::GetFileName($s); Exit = $code }
        Log ("Szenario {0}: Exit {1}" -f [IO.Path]::GetFileName($s), $code)
        if ($code -eq 3) { break }
    }
    return $ergebnisse
}

# ---------------------------------------------------------------- Ablauf
Log "Laufzeit-Arm $Ticket auf $head (Basis $Basis), Repo $Repo"
$lohnt = Lohnt-Es
Log "Lohnt es? $($lohnt.Ja) - $($lohnt.Grund)"
if (-not $lohnt.Ja) { Ende 0 'UEBERSPRUNGEN' $lohnt.Grund }

$installation = 'keine (-OhneInstallation)'
if (-not $OhneInstallation) {
    try { $inst = Installiere } catch { $inst = @{ Ok = $false; Ueberspringen = $false; Grund = $_.Exception.Message } }
    if ($inst.Ueberspringen) { Ende 0 'UEBERSPRUNGEN' $inst.Grund }
    if (-not $inst.Ok) { Log "Installation fehlgeschlagen: $($inst.Grund)"; Ende 3 'VORAUSSETZUNG' "Installation: $($inst.Grund)" }
    $installation = $inst.Grund
    $Neustart = $true
}

try {
    Stelle-LoopMidi-Sicher
    Beende-Restprozesse
    $flp = Bereite-Projekt
    $laufend = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0 -and $laufend.Count -eq 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" }
    if ($Neustart -and $laufend.Count -gt 0) { Beende-Diagnose-FL; $laufend = @() }
    if ($laufend.Count -eq 0) { $null = Starte-FL $flp; Warte-Boot } else { Log "Diagnose-FL laeuft bereits: PID $($laufend[0].Id)" }
    $ping = Warte-Ping
} catch {
    Log "Bereitschaft fehlgeschlagen: $($_.Exception.Message)"
    Ende 3 'VORAUSSETZUNG' $_.Exception.Message
}

Schreibe-Roh 'LAEUFT' "installation=$installation"
$ergebnisse = Fahre-Szenarien
$verfehlt = @($ergebnisse | Where-Object { $_.Exit -ne 0 })
$zusammen = ($ergebnisse | ForEach-Object { "{0}={1}" -f $_.Datei, $_.Exit }) -join ','

if ($Beenden) { Beende-Diagnose-FL }

$zusatz = "installation=$installation szenarien=$($ergebnisse.Count) verfehlt=$($verfehlt.Count) [$zusammen] fl=$($ping.Version)"
if ($ergebnisse | Where-Object { $_.Exit -eq 3 }) { Ende 3 'VORAUSSETZUNG' $zusatz }
if ($verfehlt.Count -gt 0) { Ende 4 'VERFEHLT' $zusatz }
Ende 0 'GEMESSEN' $zusatz
