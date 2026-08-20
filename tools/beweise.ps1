#Requires -Version 7.0
<#
.SYNOPSIS
    Nakama-Beweis-Runner - faehrt den kompletten Beweis-Kanon und schreibt die
    ROHE Ausgabe in ein Manifest unter docs/beweise/.

.DESCRIPTION
    Ersatz fuer die bewusst nicht gebaute CI (docs/bauaufteilung-sonden.md Abschnitt 1.1):
    eine Maschine, ein Entwickler, ein JUCE-Windows-Build. Ein Befehl statt zehn
    Handgriffe - das ist der Unterschied zwischen Beweisstandard und
    Beweisvorsatz.

    Der Runner fasst NICHTS zusammen: stdout und stderr jedes Laufs landen
    unveraendert im Manifest. Er misst ausserdem den BAUSTAND - sind die
    Pruefbinaries aelter als die Quellen, verweigert er die Beglaubigung
    (Exitcode 4), statt eine veraltete Messung als Basislinie auszugeben.

    Der Kanon waechst mit dem Plan: geplante Pruefbinaries (ab P0/P1/P2/P6)
    stehen bereits in der Tabelle. Solange sie fehlen, sind sie eine neutrale
    Zeile "geplant"; sobald sie gebaut sind, laufen sie automatisch mit und
    sind ab dann Pflicht.

.PARAMETER Ziel
    Zieldatei des Manifests. Vorgabe: docs/beweise/lauf-<zeit>.md

.PARAMETER Anhaengen
    Haengt den Lauf an ein bestehendes Ticket-Manifest an, statt es zu ersetzen.

.PARAMETER Bauen
    Baut die Kanon-Ziele vorher neu (Release), damit der Lauf den aktuellen
    Quellstand beweist.

.PARAMETER Titel
    Ueberschrift des Manifests bzw. des angehaengten Abschnitts.

.EXAMPLE
    pwsh -File tools/beweise.ps1 -Ziel docs/beweise/S0-basislinie.md -Titel 'S0 | Basislinie'

.EXAMPLE
    pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-004.md -Anhaengen

.NOTES
    Exitcodes
      0  alles gruen
      2  mindestens ein Kanon-Lauf rot
      3  Voraussetzung fehlt (nicht gebaut, keine Fixtures, kein cargo)
      4  Laeufe gruen, aber Binaries aelter als die Quellen (nicht beglaubigt)
    Reihenfolge der Beurteilung: 2 vor 3 vor 4.
#>

[CmdletBinding()]
param(
    [string] $Ziel,
    [switch] $Anhaengen,
    [switch] $Bauen,
    [string] $Titel = 'Kanon-Lauf'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# Native Befehle duerfen mit Exit != 0 zurueckkommen, ohne das Skript zu toeten -
# genau das ist ja der Messwert (PS 7.4+ wuerfe sonst bei ErrorActionPreference=Stop).
$PSNativeCommandUseErrorActionPreference = $false

$Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Beginn = Get-Date

# ---------------------------------------------------------------- Hilfsmittel

function Argument-Quoten {
    <# Start-Process -ArgumentList klebt die Elemente UNQUOTIERT mit Leerzeichen
       zusammen (anders als ProcessStartInfo.ArgumentList in .NET). Ohne diese
       Quotierung zerfiele jeder Pfad mit Leerzeichen still in zwei Argumente -
       gemessen an "git log --format=%h %s", das dabei Exit 128 lieferte. #>
    param([string] $Argument)
    if ($null -eq $Argument -or $Argument -eq '') { return '""' }
    if ($Argument -notmatch '[\s"]') { return $Argument }
    # Windows-Kommandozeilenregeln: Backslashes vor einem " (und am Ende) verdoppeln.
    $s = [regex]::Replace($Argument, '(\\*)"', '$1$1\"')
    $s = [regex]::Replace($s, '(\\+)$', '$1$1')
    return '"' + $s + '"'
}

function Fuehre-Aus {
    <# Fuehrt ein Programm aus und liefert Exitcode, stdout, stderr und Dauer.
       stdout/stderr gehen getrennt in Temp-Dateien: kein PowerShell-Fehlerstrom
       dazwischen, keine ErrorRecords, keine Verstuemmelung. #>
    param(
        [Parameter(Mandatory)][string] $Datei,
        [string[]] $Argumente = @(),
        [string] $Arbeitsverzeichnis = $Wurzel
    )

    $ausDatei = [IO.Path]::GetTempFileName()
    $errDatei = [IO.Path]::GetTempFileName()
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $code = -1
    $fehlertext = ''

    try {
        $start = @{
            FilePath               = $Datei
            NoNewWindow            = $true
            Wait                   = $true
            PassThru               = $true
            RedirectStandardOutput = $ausDatei
            RedirectStandardError  = $errDatei
            WorkingDirectory       = $Arbeitsverzeichnis
        }
        if ($Argumente.Count -gt 0) { $start.ArgumentList = @($Argumente | ForEach-Object { Argument-Quoten $_ }) }

        $prozess = Start-Process @start
        if ($null -ne $prozess) {
            $prozess.WaitForExit()
            $code = $prozess.ExitCode
        }
    }
    catch {
        $fehlertext = $_.Exception.Message
    }

    $uhr.Stop()
    $aus = ''
    $err = ''
    if (Test-Path -LiteralPath $ausDatei) { $aus = (Get-Content -LiteralPath $ausDatei -Raw -Encoding utf8) ?? '' }
    if (Test-Path -LiteralPath $errDatei) { $err = (Get-Content -LiteralPath $errDatei -Raw -Encoding utf8) ?? '' }
    if ($fehlertext) { $err = ($err + "`n[Start-Process] " + $fehlertext).Trim() }
    Remove-Item -LiteralPath $ausDatei, $errDatei -Force -ErrorAction SilentlyContinue

    [pscustomobject]@{
        ExitCode = $code
        StdOut   = $aus
        StdErr   = $err
        Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 2)
    }
}

function Einzeilig {
    <# Erste Ausgabezeile eines Programms - fuer Versionsabfragen. #>
    param([string] $Datei, [string[]] $Argumente = @())
    if (-not (Get-Command $Datei -ErrorAction SilentlyContinue) -and -not (Test-Path -LiteralPath $Datei)) {
        return 'nicht gefunden'
    }
    $r = Fuehre-Aus -Datei $Datei -Argumente $Argumente
    $text = ($r.StdOut + "`n" + $r.StdErr).Trim()
    if (-not $text) { return 'nicht ermittelbar' }
    return ($text -split "`r?`n")[0].Trim()
}

function Git-Wert {
    param([string[]] $Argumente)
    $r = Fuehre-Aus -Datei 'git' -Argumente (@('-C', $Wurzel) + $Argumente)
    if ($r.ExitCode -ne 0) { return 'nicht ermittelbar' }
    return $r.StdOut.Trim()
}

function Zellentext {
    <# Markdown-Tabellenzellen vertragen kein rohes | und keine Zeilenumbrueche. #>
    param([string] $Text)
    if ($null -eq $Text) { return '' }
    return ($Text -replace '\|', '\|' -replace '\r?\n', ' ').Trim()
}

function Zaun {
    <# Waehlt einen Codezaun, der laenger ist als jede Zaunfolge im Inhalt -
       sonst zerreisst eine Testausgabe mit dreifachem Backtick das Manifest. #>
    param([string] $Inhalt)
    $laenge = 3
    foreach ($treffer in [regex]::Matches(($Inhalt ?? ''), '`{3,}')) {
        if ($treffer.Value.Length -ge $laenge) { $laenge = $treffer.Value.Length + 1 }
    }
    return ('`' * $laenge)
}

function Block {
    <# Rohausgabe als Codeblock - unveraendert, nur eingezaeunt. #>
    param([string] $Inhalt, [string] $Sprache = 'text')
    if ([string]::IsNullOrWhiteSpace($Inhalt)) { return '_(leer)_' }
    $zaun = Zaun $Inhalt
    return ($zaun + $Sprache + "`n" + $Inhalt.TrimEnd() + "`n" + $zaun)
}

function Dauertext {
    <# Eine Schreibweise fuer Konsole und Manifest (Kultur der Maschine). #>
    param([double] $Sekunden)
    return ('{0:N2} s' -f $Sekunden)
}

function Pruefbinaer {
    param([string] $Name)
    return (Join-Path $Wurzel ("eq-copilot\build\plugin\{0}_artefacts\Release\{0}.exe" -f $Name))
}

function RelativZurWurzel {
    param([string] $Pfad)
    if ($Pfad.StartsWith($Wurzel, [StringComparison]::OrdinalIgnoreCase)) {
        return $Pfad.Substring($Wurzel.Length).TrimStart('\', '/')
    }
    return $Pfad
}

# ------------------------------------------------------------------ Der Kanon

$fixtureVerzeichnis = Join-Path $Wurzel 'eq-copilot\fixtures'

$kanon = @(
    [pscustomobject]@{
        Kuerzel    = 'A1'
        Name       = 'EqCopNullTest'
        Art        = 'plugin'
        Argumente  = @()
        AbPhase    = 'jetzt'
        Behauptung = 'Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert.'
    }
    [pscustomobject]@{
        Kuerzel    = 'A2'
        Name       = 'EqCopGoldenTest'
        Art        = 'plugin'
        Argumente  = @($fixtureVerzeichnis)
        AbPhase    = 'jetzt'
        Behauptung = 'AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel).'
    }
    [pscustomobject]@{
        Kuerzel    = 'A3'
        Name       = 'EqCopMarkierungTest'
        Art        = 'plugin'
        Argumente  = @()
        AbPhase    = 'jetzt'
        Behauptung = 'Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung.'
    }
    [pscustomobject]@{
        Kuerzel    = 'A4'
        Name       = 'broker'
        Art        = 'cargo'
        Argumente  = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never')
        AbPhase    = 'jetzt'
        Behauptung = 'Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server).'
    }
    # --- geplant: laufen automatisch mit, sobald sie gebaut sind -------------
    [pscustomobject]@{ Kuerzel='B1'; Name='EqCopIdentityTest';       Art='plugin'; Argumente=@(); AbPhase='P0'; Behauptung='Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren.' }
    [pscustomobject]@{ Kuerzel='B2'; Name='EqCopStateMigrationTest'; Art='plugin'; Argumente=@(); AbPhase='P1'; Behauptung='State-Schema 2 laedt reine Schema-1-Staende ohne Verlust.' }
    # B3 wurde in P0 gebaut (SONDE-003, Bridge-Patch), nicht erst in P2 - die
    # Phase ist hier berichtigt, damit die Zeile nicht laenger "geplant" heisst,
    # waehrend das Ziel schon Pflicht ist.
    [pscustomobject]@{ Kuerzel='B3'; Name='EqCopHostContextTest';    Art='plugin'; Argumente=@(); AbPhase='P0'; Behauptung='Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen.' }
    [pscustomobject]@{ Kuerzel='B4'; Name='EqCopQueueStressTest';    Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus.' }
    [pscustomobject]@{ Kuerzel='B5'; Name='EqCopAnalysisGoldenTest'; Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege.' }
    [pscustomobject]@{ Kuerzel='B6'; Name='EqCopDspGoldenTest';      Art='plugin'; Argumente=@(); AbPhase='P6'; Behauptung='Aktiver DSP-Kern liefert die eingefrorene Referenzantwort.' }
    [pscustomobject]@{ Kuerzel='B7'; Name='EqCopTransactionTest';    Art='plugin'; Argumente=@(); AbPhase='P6'; Behauptung='Apply/Revert ist transaktional - kein halber Zustand ueberlebt.' }
)

# Ziele, die nicht selbst im Kanon laufen, aber von einem Kanon-Lauf GEMESSEN
# werden. Ohne sie prueft der Test ein altes Artefakt und bleibt gruen, waehrend
# die Quelle laengst etwas anderes sagt (T2-Befund vom 20.08.).
$gemesseneZiele = @(
    [pscustomobject]@{
        Ziel   = 'EqCopilot_VST3'
        Marker = 'juce_add_plugin(EqCopilot'
        Wegen  = 'EqCopIdentityTest misst dessen moduleinfo.json'
    }
)

# ------------------------------------------------------ Optional: vorher bauen

$cmakeBefehl = $null
$bauProtokoll = @()

function Finde-CMake {
    $ausPfad = Get-Command cmake -ErrorAction SilentlyContinue
    if ($ausPfad) { return $ausPfad.Source }
    $kandidaten = @()
    foreach ($stamm in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
        if (-not $stamm) { continue }
        foreach ($ausgabe in @('BuildTools', 'Community', 'Professional', 'Enterprise')) {
            $kandidaten += (Join-Path $stamm "Microsoft Visual Studio\2022\$ausgabe\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
        }
        $kandidaten += (Join-Path $stamm 'CMake\bin\cmake.exe')
    }
    foreach ($k in $kandidaten) { if (Test-Path -LiteralPath $k) { return $k } }
    return $null
}

function Bau-Abbruch {
    <# Ein gescheiterter Bau darf seine Ursache nicht mitnehmen. Die rohe
       Ausgabe wandert neben das Manifest, die Fehlerzeilen zusaetzlich auf
       die Konsole - sonst steht da nur "Exit 1" und niemand weiss warum.
       (Selbst erlebt am 20.08.: der Runner verschluckte den Compilerfehler.) #>
    param([string] $Schritt, [pscustomobject] $Lauf)

    $roh = ($Lauf.StdOut + "`n" + $Lauf.StdErr).Trim()
    $logDatei = Join-Path $Wurzel ('tmp\bau-fehler-{0}.log' -f $Beginn.ToString('yyyy-MM-dd-HHmm'))
    $logOrdner = Split-Path -Parent $logDatei
    if (-not (Test-Path -LiteralPath $logOrdner)) { New-Item -ItemType Directory -Path $logOrdner -Force | Out-Null }
    Set-Content -LiteralPath $logDatei -Value $roh -Encoding utf8

    $fehlerzeilen = @($roh -split "`r?`n" | Where-Object { $_ -match 'error|fehler' } | Select-Object -First 12)
    Write-Host ''
    Write-Host ("Bau ({0}) fehlgeschlagen - Exit {1}. Die ersten Fehlerzeilen:" -f $Schritt, $Lauf.ExitCode) -ForegroundColor Red
    foreach ($zeile in $fehlerzeilen) { Write-Host ("  " + $zeile.Trim()) -ForegroundColor Red }
    Write-Host ("Vollstaendiges Bauprotokoll: {0}" -f (RelativZurWurzel $logDatei)) -ForegroundColor Yellow
    Write-Host ''

    throw ("Bau fehlgeschlagen (Exit {0}) - kein Beweislauf auf kaputtem Baustand. Protokoll: {1}" `
           -f $Lauf.ExitCode, (RelativZurWurzel $logDatei))
}

if ($Bauen) {
    $cmakeBefehl = Finde-CMake
    if (-not $cmakeBefehl) {
        throw 'cmake nicht gefunden - weder im PATH noch unter Visual Studio 2022. -Bauen ist ohne cmake nicht moeglich.'
    }

    $bauVerzeichnis = Join-Path $Wurzel 'eq-copilot\build'
    $loesung = Join-Path $bauVerzeichnis 'EqCopilotSuite.sln'
    if (-not (Test-Path -LiteralPath $loesung)) {
        Write-Host 'Konfiguriere (erstmalig) ...' -ForegroundColor DarkGray
        $k = Fuehre-Aus -Datei $cmakeBefehl -Argumente @('-S', 'eq-copilot', '-B', 'eq-copilot/build', '-G', 'Visual Studio 17 2022', '-A', 'x64')
        $bauProtokoll += [pscustomobject]@{ Schritt = 'configure'; ExitCode = $k.ExitCode; StdOut = $k.StdOut; StdErr = $k.StdErr; Sekunden = $k.Sekunden }
        if ($k.ExitCode -ne 0) { Bau-Abbruch -Schritt 'configure' -Lauf $k }
    }

    # Nur Ziele bauen, die es im CMakeLists wirklich gibt (geplante Tests noch nicht).
    $cmakeText = Get-Content -LiteralPath (Join-Path $Wurzel 'eq-copilot\plugin\CMakeLists.txt') -Raw
    $zuBauen = @($kanon | Where-Object { $_.Art -eq 'plugin' -and $cmakeText -match [regex]::Escape($_.Name) } | ForEach-Object { $_.Name })
    $zuBauen += @($gemesseneZiele | Where-Object { $cmakeText -match [regex]::Escape($_.Marker) } | ForEach-Object { $_.Ziel })
    Write-Host ('Baue: ' + ($zuBauen -join ', ')) -ForegroundColor DarkGray
    $b = Fuehre-Aus -Datei $cmakeBefehl -Argumente (@('--build', 'eq-copilot/build', '--config', 'Release', '--target') + $zuBauen)
    $bauProtokoll += [pscustomobject]@{ Schritt = 'build'; ExitCode = $b.ExitCode; StdOut = $b.StdOut; StdErr = $b.StdErr; Sekunden = $b.Sekunden }
    if ($b.ExitCode -ne 0) { Bau-Abbruch -Schritt 'build' -Lauf $b }
}

# ------------------------------------------------------------------ Kopfdaten

$juceGepinnt = 'nicht ermittelbar'
$cmakeWurzelDatei = Join-Path $Wurzel 'eq-copilot\CMakeLists.txt'
if (Test-Path -LiteralPath $cmakeWurzelDatei) {
    $treffer = Select-String -LiteralPath $cmakeWurzelDatei -Pattern 'GIT_TAG\s+(\S+)' | Select-Object -First 1
    if ($treffer) { $juceGepinnt = $treffer.Matches[0].Groups[1].Value }
}

$juceQuelle = Join-Path $Wurzel 'eq-copilot\build\_deps\juce-src'
$jucePlatte = 'nicht gebaut'
if (Test-Path -LiteralPath $juceQuelle) {
    $r = Fuehre-Aus -Datei 'git' -Argumente @('-C', $juceQuelle, 'describe', '--tags', '--always', '--dirty')
    $jucePlatte = if ($r.ExitCode -eq 0 -and $r.StdOut.Trim()) { $r.StdOut.Trim() } else { 'vorhanden, Version nicht ermittelbar' }
}

$flVersionen = @()
$ilStamm = 'C:\Program Files\Image-Line'
if (Test-Path -LiteralPath $ilStamm) {
    foreach ($ordner in (Get-ChildItem -LiteralPath $ilStamm -Directory -Filter 'FL Studio *' -ErrorAction SilentlyContinue)) {
        $flExe = Join-Path $ordner.FullName 'FL64.exe'
        if (Test-Path -LiteralPath $flExe) {
            $flVersionen += ('{0} {1}' -f $ordner.Name, (Get-Item -LiteralPath $flExe).VersionInfo.ProductVersion)
        }
    }
}
if ($flVersionen.Count -eq 0) { $flVersionen = @('keine FL-Installation gefunden') }

$schmutzig = Git-Wert @('status', '--porcelain')
$schmutzigeDateien = @()
if ($schmutzig -and $schmutzig -ne 'nicht ermittelbar') {
    $schmutzigeDateien = @($schmutzig -split "`r?`n" | Where-Object { $_.Trim() })
}

$kopf = [ordered]@{
    'Zeitpunkt'       = $Beginn.ToString('yyyy-MM-dd HH:mm:ss zzz')
    'Rechner'         = ('{0} | Windows {1}' -f $env:COMPUTERNAME, [Environment]::OSVersion.Version.ToString())
    'Zweig'           = (Git-Wert @('rev-parse', '--abbrev-ref', 'HEAD'))
    'Commit'          = (Git-Wert @('log', '-1', '--format=%h %s'))
    'Commit (voll)'   = (Git-Wert @('rev-parse', 'HEAD'))
    'Arbeitsbaum'     = $(if ($schmutzigeDateien.Count -eq 0) { 'sauber' } else { '{0} unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit' -f $schmutzigeDateien.Count })
    'JUCE gepinnt'    = $juceGepinnt
    'JUCE auf Platte' = $jucePlatte
    'FL Studio'       = ($flVersionen -join ' | ')
    'cargo'           = (Einzeilig 'cargo' @('--version'))
    'rustc'           = (Einzeilig 'rustc' @('--version'))
    'PowerShell'      = $PSVersionTable.PSVersion.ToString()
}
if ($Bauen -and $cmakeBefehl) { $kopf['cmake'] = (Einzeilig $cmakeBefehl @('--version')) }

# ------------------------------------------------------------------ Baustand

# Vollstaendig halten: JEDE Quelle, aus der eine Pruefbinaerdatei entsteht.
# Fehlt ein Ort, beglaubigt der Runner eine veraltete Messung als frisch -
# genau der T2-Befund vom 20.08., nur eine Ebene tiefer. hostbridge/, spike/,
# probe/, cmake/ und der JUCE-Bridge-Patch kamen mit SONDE-003/004a dazu.
$quellOrte = @(
    (Join-Path $Wurzel 'eq-copilot\plugin\src'),
    (Join-Path $Wurzel 'eq-copilot\plugin\tests'),
    (Join-Path $Wurzel 'eq-copilot\plugin\hostbridge'),
    (Join-Path $Wurzel 'eq-copilot\plugin\spike'),
    (Join-Path $Wurzel 'eq-copilot\plugin\probe'),
    (Join-Path $Wurzel 'eq-copilot\cmake'),
    (Join-Path $Wurzel 'third_party\patches'),
    (Join-Path $Wurzel 'eq-copilot\plugin\CMakeLists.txt'),
    (Join-Path $Wurzel 'eq-copilot\CMakeLists.txt')
)
$neuesteQuelle = $null
foreach ($ort in $quellOrte) {
    if (-not (Test-Path -LiteralPath $ort)) { continue }
    $dateien = if ((Get-Item -LiteralPath $ort) -is [IO.DirectoryInfo]) {
        Get-ChildItem -LiteralPath $ort -Recurse -File -ErrorAction SilentlyContinue
    } else {
        @(Get-Item -LiteralPath $ort)
    }
    foreach ($d in $dateien) {
        if ($null -eq $neuesteQuelle -or $d.LastWriteTime -gt $neuesteQuelle) { $neuesteQuelle = $d.LastWriteTime }
    }
}

# Hat -Bauen soeben erfolgreich gebaut, ist die Frage entschieden: das
# Buildsystem hat die Abhaengigkeiten geprueft, und sein Urteil schlaegt den
# Zeitstempelvergleich. Ohne -Bauen bleibt die mtime die ehrliche Heuristik.
# (Sonst meldete jede fremde Datei im tests-Ordner die Binaries dauerhaft als
#  veraltet, obwohl ein No-op-Bau die Zeitstempel gar nicht anfasst.)
$bauBestaetigt = $Bauen -and (@($bauProtokoll | Where-Object { $_.Schritt -eq 'build' -and $_.ExitCode -eq 0 }).Count -gt 0)

$baustand = @()
$veraltet = $false
foreach ($eintrag in ($kanon | Where-Object { $_.Art -eq 'plugin' })) {
    $exe = Pruefbinaer $eintrag.Name
    if (-not (Test-Path -LiteralPath $exe)) { continue }
    $datei = Get-Item -LiteralPath $exe
    $istVeraltet = (-not $bauBestaetigt) `
                   -and ($null -ne $neuesteQuelle -and $datei.LastWriteTime -lt $neuesteQuelle)
    if ($istVeraltet) { $veraltet = $true }
    $baustand += [pscustomobject]@{
        Name   = $eintrag.Name
        Gebaut = $datei.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss')
        Hash   = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.Substring(0, 16)
        Stand  = $(if ($istVeraltet) { 'VERALTET' } elseif ($bauBestaetigt) { 'frisch (Bau bestaetigt)' } else { 'frisch' })
    }
}

# --------------------------------------------------------------- Kanon fahren

$ergebnisse = @()
$rot = 0
$fehlendeVoraussetzung = 0

foreach ($eintrag in $kanon) {

    $zeile = [pscustomobject]@{
        Kuerzel    = $eintrag.Kuerzel
        Name       = $eintrag.Name
        Behauptung = $eintrag.Behauptung
        Befehl     = ''
        Status     = ''
        Symbol     = ''
        ExitCode   = $null
        Sekunden   = $null
        StdOut     = ''
        StdErr     = ''
        Gelaufen   = $false
    }

    if ($eintrag.Art -eq 'cargo') {
        $zeile.Befehl = 'cargo ' + ($eintrag.Argumente -join ' ')
        if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
            $zeile.Symbol = '[FEHLT]'
            $zeile.Status = 'cargo nicht gefunden'
            $fehlendeVoraussetzung++
            $ergebnisse += $zeile
            Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            continue
        }
        $lauf = Fuehre-Aus -Datei 'cargo' -Argumente $eintrag.Argumente
    }
    else {
        $exe = Pruefbinaer $eintrag.Name
        $zeile.Befehl = (RelativZurWurzel $exe) + $(if ($eintrag.Argumente.Count -gt 0) { ' ' + (($eintrag.Argumente | ForEach-Object { RelativZurWurzel $_ }) -join ' ') } else { '' })

        if (-not (Test-Path -LiteralPath $exe)) {
            if ($eintrag.AbPhase -eq 'jetzt') {
                $zeile.Symbol = '[FEHLT]'
                $zeile.Status = 'nicht gebaut - mit -Bauen nachziehen'
                $fehlendeVoraussetzung++
                Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            }
            else {
                $zeile.Symbol = '[GEPLANT]'
                $zeile.Status = ('geplant (ab {0})' -f $eintrag.AbPhase)
            }
            $ergebnisse += $zeile
            continue
        }

        # Golden braucht die nicht eingecheckten Fixture-WAVs.
        if ($eintrag.Name -eq 'EqCopGoldenTest') {
            $wavs = @(Get-ChildItem -LiteralPath $fixtureVerzeichnis -Filter '*.wav' -ErrorAction SilentlyContinue)
            if ($wavs.Count -eq 0) {
                $zeile.Symbol = '[FEHLT]'
                $zeile.Status = 'Fixture-WAVs fehlen - py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav'
                $fehlendeVoraussetzung++
                $ergebnisse += $zeile
                Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
                continue
            }
        }

        $lauf = Fuehre-Aus -Datei $exe -Argumente $eintrag.Argumente
    }

    $zeile.Gelaufen = $true
    $zeile.ExitCode = $lauf.ExitCode
    $zeile.Sekunden = $lauf.Sekunden
    $zeile.StdOut = $lauf.StdOut
    $zeile.StdErr = $lauf.StdErr
    if ($lauf.ExitCode -eq 0) {
        $zeile.Symbol = '[OK]'
        $zeile.Status = 'Exit 0'
        Write-Host ('[OK] {0} - Exit 0 ({1})' -f $zeile.Name, (Dauertext $zeile.Sekunden)) -ForegroundColor Green
    }
    else {
        $zeile.Symbol = '[ROT]'
        $zeile.Status = ('Exit {0}' -f $lauf.ExitCode)
        $rot++
        Write-Host ('[ROT] {0} - Exit {1} ({2})' -f $zeile.Name, $lauf.ExitCode, (Dauertext $zeile.Sekunden)) -ForegroundColor Red
    }

    $ergebnisse += $zeile
}

# -------------------------------------------------------------------- Urteil

$gelaufen = @($ergebnisse | Where-Object { $_.Gelaufen })
$gruen = @($gelaufen | Where-Object { $_.ExitCode -eq 0 })
$geplant = @($ergebnisse | Where-Object { $_.Symbol -eq '[GEPLANT]' })
# Ein "4/4 gruen" waere geschoenigt, solange sieben Kanon-Eintraege nur geplant sind.
$nachsatz = if ($geplant.Count -gt 0) { " | $($geplant.Count) geplante Pruefung(en) noch nicht gebaut" } else { '' }

$exitcode = 0
if ($rot -gt 0) {
    $exitcode = 2
    $urteil = "ROT - $rot von $($gelaufen.Count) Kanon-Laeufen fehlgeschlagen$nachsatz"
}
elseif ($fehlendeVoraussetzung -gt 0) {
    $exitcode = 3
    $urteil = "UNVOLLSTAENDIG - $($gruen.Count) gruen, $fehlendeVoraussetzung Voraussetzung(en) fehlen$nachsatz"
}
elseif ($veraltet) {
    $exitcode = 4
    $urteil = "NICHT BEGLAUBIGT - $($gruen.Count)/$($gelaufen.Count) gruen, aber Pruefbinaries sind aelter als die Quellen$nachsatz"
}
else {
    $urteil = "GRUEN - $($gruen.Count)/$($gelaufen.Count) Kanon-Laeufe bestanden$nachsatz"
}

# ------------------------------------------------------------------ Manifest

if (-not $Ziel) {
    $Ziel = Join-Path $Wurzel ('docs\beweise\lauf-{0}.md' -f $Beginn.ToString('yyyy-MM-dd-HHmm'))
}
elseif (-not [IO.Path]::IsPathRooted($Ziel)) {
    $Ziel = Join-Path $Wurzel $Ziel
}
$zielVerzeichnis = Split-Path -Parent $Ziel
if (-not (Test-Path -LiteralPath $zielVerzeichnis)) { New-Item -ItemType Directory -Path $zielVerzeichnis -Force | Out-Null }

$z = [Collections.Generic.List[string]]::new()

if ($Anhaengen -and (Test-Path -LiteralPath $Ziel)) {
    $z.Add('')
    $z.Add('---')
    $z.Add('')
    $z.Add("## Kanon-Lauf - $Titel")
}
else {
    $z.Add("# Beweismanifest - $Titel")
}
$z.Add('')
$z.Add("**Lauf:** $($Beginn.ToString('yyyy-MM-dd HH:mm')) | **Runner:** ``tools/beweise.ps1`` | **Urteil:** $urteil | **Exitcode:** $exitcode")
$z.Add('')
$z.Add('### Kopf - woran gemessen wurde')
$z.Add('')
$z.Add('| Feld | Wert |')
$z.Add('|---|---|')
foreach ($schluessel in $kopf.Keys) { $z.Add("| $schluessel | $(Zellentext $kopf[$schluessel]) |") }
$z.Add('')

if ($schmutzigeDateien.Count -gt 0) {
    $z.Add('<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>')
    $z.Add('')
    $z.Add((Block ($schmutzigeDateien -join "`n")))
    $z.Add('')
    $z.Add('</details>')
    $z.Add('')
}

$z.Add('### Baustand der Pruefbinaries')
$z.Add('')
if ($baustand.Count -eq 0) {
    $z.Add('_Keine Pruefbinaries vorhanden._')
}
else {
    $z.Add('| Binaerdatei | gebaut am | SHA-256 (16) | Stand |')
    $z.Add('|---|---|---|---|')
    foreach ($b in $baustand) { $z.Add("| ``$($b.Name)`` | $($b.Gebaut) | ``$($b.Hash)`` | $($b.Stand) |") }
    $z.Add('')
    $z.Add("Neueste Quelldatei (``plugin/src``, ``tests``, ``hostbridge``, ``spike``, ``probe``, ``cmake``, ``third_party/patches``, CMakeLists): **$(if ($neuesteQuelle) { $neuesteQuelle.ToString('yyyy-MM-dd HH:mm:ss') } else { 'nicht ermittelbar' })**. ``cargo test`` uebersetzt selbst und ist damit immer frisch.")
    if ($bauBestaetigt) {
        $z.Add('')
        $z.Add('Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.')
    }
}
$z.Add('')
if ($veraltet) {
    $z.Add('> **VERALTET - dieser Lauf beweist NICHT den aktuellen Quellstand.**')
    $z.Add('> Mindestens eine Pruefbinaerdatei ist aelter als die Quellen. Neu fahren mit `-Bauen`.')
    $z.Add('')
}

$z.Add('### Uebersicht')
$z.Add('')
$z.Add('| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |')
$z.Add('|---|---|---|---|---|---|')
foreach ($e in $ergebnisse) {
    $dauer = if ($null -ne $e.Sekunden) { Dauertext $e.Sekunden } else { '-' }
    $link = if ($e.Gelaufen) { "[↓ $($e.Kuerzel)](#$($e.Kuerzel.ToLower()))" } else { '-' }
    $z.Add("| $($e.Kuerzel) | $(Zellentext $e.Behauptung) | ``$(Zellentext $e.Befehl)`` | $($e.Symbol) $(Zellentext $e.Status) | $dauer | $link |")
}
$z.Add('')

$z.Add('### Rohe Ausgaben')
$z.Add('')
foreach ($e in ($ergebnisse | Where-Object { $_.Gelaufen })) {
    $z.Add("<a id=`"$($e.Kuerzel.ToLower())`"></a>")
    $z.Add("#### $($e.Kuerzel) | $($e.Name)")
    $z.Add('')
    $z.Add("**Befehl:** ``$(Zellentext $e.Befehl)`` | **Exitcode:** $($e.ExitCode) | **Dauer:** $(Dauertext $e.Sekunden)")
    $z.Add('')
    $z.Add('stdout:')
    $z.Add('')
    $z.Add((Block $e.StdOut))
    $z.Add('')
    $z.Add('stderr:')
    $z.Add('')
    $z.Add((Block $e.StdErr))
    $z.Add('')
}

if ($bauProtokoll.Count -gt 0) {
    $z.Add('### Bau vor dem Lauf (`-Bauen`)')
    $z.Add('')
    foreach ($b in $bauProtokoll) {
        $z.Add("**$($b.Schritt)** | Exit $($b.ExitCode) | $(Dauertext $b.Sekunden)")
        $z.Add('')
        $z.Add('<details><summary>Rohe Ausgabe</summary>')
        $z.Add('')
        $z.Add((Block ($b.StdOut + "`n" + $b.StdErr)))
        $z.Add('')
        $z.Add('</details>')
        $z.Add('')
    }
}

$inhalt = ($z -join "`n")
if ($Anhaengen -and (Test-Path -LiteralPath $Ziel)) {
    Add-Content -LiteralPath $Ziel -Value $inhalt -Encoding utf8
}
else {
    Set-Content -LiteralPath $Ziel -Value $inhalt -Encoding utf8
}

Write-Host ''
Write-Host $urteil
Write-Host ('Manifest: {0}' -f (RelativZurWurzel $Ziel))
exit $exitcode
