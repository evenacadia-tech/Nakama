#requires -Version 7.2
<#
Gesamtplan dauerhaft im zweiten Terminal-Tab (NAK-256). User-Entscheid 11.09.2026:
„gesamtplan stand dauerhaft im zweiten tab + statuszeile".

Zwei Ansichten im selben Tab. Die zweite ist das Zielbild (User-Auftrag 23.09.2026: „ein
sich stets aktualisierendes standbild in form einer übersichtlichen beschreibung was nakama
aus sicht eines potentiellen users können würde"):

  Plan      oben STAND, darunter der Gesamtplan (beides aus `cockpit.ps1 -Uebersicht`, bei zu
            kleinem Fenster mit gefalteten fertigen Phasen).
  Zielbild  wie Nakama arbeitet, wenn der heutige Plan fertig ist: `docs/ZIELBILD.md`,
            gezeichnet von `tools/plan/zielbild.py anzeige`. Die Kopfzeile steht fest und
            nennt, ob alle Entscheide eingearbeitet sind; der Text darunter wird geblaettert.

Tasten: Z Zielbild · P Plan · Tab wechselt. Im Zielbild ↑ ↓ eine Zeile, Bild↑ Bild↓ und
Leertaste eine Seite, Pos1 und Ende. Das Mausrad blaettert ebenfalls: der Tab zeichnet auf
dem alternativen Bildschirm mit DECSET 1007, dort schickt Windows Terminal das Rad als
Pfeiltasten.

Im Takt von -TaktSekunden wird die gewaehlte Ansicht neu gerechnet, unten stehen die Uhrzeit
des Zeichnens sowie Kontext und Kontingente aus dem Telemetrie-Cache der
Dirigenten-Statuszeile. Rein lesend: der Tab schreibt weder eine Projektdatei noch einen Cache.

Neue Fassung: aendert sich diese Datei auf der Platte (etwa durch einen Pull) und parst sie
fehlerfrei, startet der Tab sie beim naechsten Takt im selben Tab neu; eine fehlerhafte
Fassung meldet er und laeuft mit der alten weiter. Aenderungen am Tab brauchen damit keinen
Fensterneustart (vgl. NAK-257 fuer den Starter).

-Einmal    zeichnet genau einmal, ohne Bildschirmsteuerung und ohne Tasten (Probe,
           Umleitung); das Zielbild dann vollstaendig statt seitenweise und ohne Farben.
-Ansicht   Plan (Standard) oder Zielbild: die Ansicht beim Start und bei -Einmal.
-BindenAn  Prozess, mit dessen Ende der Tab sauber endet (Exit 0, Windows Terminal
           schliesst den Tab); der Starter bindet ihn an sein Dirigentenfenster.
Ein zweiter Tab in derselben Anmeldung endet sofort (benannter Mutex).
#>
[CmdletBinding()]
param(
    [ValidateRange(5, 3600)]
    [int]$TaktSekunden = 30,

    [switch]$Einmal,

    [ValidateRange(0, 2147483647)]
    [int]$BindenAn = 0,

    [ValidateSet('Plan', 'Zielbild')]
    [string]$Ansicht = 'Plan'
)

$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8

$cockpitPath = Join-Path $PSScriptRoot 'cockpit.ps1'
$zielbildPath = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\plan\zielbild.py'))
$powerShellPath = (Get-Process -Id $PID).Path
$pyPath = $null
try { $pyPath = (Get-Command py.exe -ErrorAction Stop).Source } catch { }
# Steuersequenzen zaehlen beim Kuerzen nicht als sichtbare Zeichen.
$ansiMuster = "`e\[[0-9;?]*[A-Za-z]"

function Format-Prozent {
    param([object]$Wert)
    if ($null -eq $Wert) { return 'nicht verfügbar' }
    return ('{0:0} %' -f [double]$Wert)
}

function Get-TelemetrieText {
    <# Kontext und Kontingente aus dem juengsten Telemetrie-Cache, den
       `cockpit.ps1 -StatusLine` der Dirigenten-Session bei jeder Aktualisierung
       schreibt. Aelter als zehn Minuten gilt als nicht verfuegbar. #>
    try {
        $datei = @(Get-ChildItem -LiteralPath ([IO.Path]::GetTempPath()) -Filter 'nakama-dirigent-*-telemetry.json' -File -ErrorAction Stop |
            Sort-Object LastWriteTimeUtc -Descending) | Select-Object -First 1
        if ($null -eq $datei) {
            return 'Kontext und Kontingente nicht verfügbar · keine Messung der Dirigenten-Statuszeile'
        }
        $daten = [IO.File]::ReadAllText($datei.FullName, [Text.Encoding]::UTF8) | ConvertFrom-Json
        if ($null -eq $daten.At) { return 'Kontext und Kontingente nicht lesbar' }
        $alter = [math]::Max(0, [DateTimeOffset]::UtcNow.ToUnixTimeSeconds() - [long]$daten.At)
        if ($alter -gt 600) {
            return "Kontext und Kontingente nicht verfügbar · letzte Messung vor $([math]::Floor($alter / 60)) min"
        }
        $kontext = if ($daten.ContextAvailable) { "Dirigent-Kontext $(Format-Prozent $daten.ContextPercent)" } else { 'Dirigent-Kontext nicht verfügbar' }
        $claude = if ($daten.ClaudeAvailable) {
            "Claude 5 Std. $(Format-Prozent $daten.ClaudeFiveUsed), Woche $(Format-Prozent $daten.ClaudeSevenUsed)"
        } else { 'Claude nicht verfügbar' }
        $codex = if ($daten.CodexAvailable) { "Codex Woche $(Format-Prozent $daten.CodexSevenUsed)" } else { 'Codex nicht verfügbar' }
        return "$kontext · $claude · $codex · gemessen vor $alter s"
    }
    catch {
        return 'Kontext und Kontingente nicht lesbar'
    }
}

function Get-Fusszeile {
    return "aktualisiert $((Get-Date).ToString('HH:mm:ss')) · $(Get-TelemetrieText)"
}

function Invoke-Zielbild {
    <# tools/plan/zielbild.py in eigenem Prozess: ein Fehler dort beendet den Tab nicht. #>
    param([string[]]$Argumente)
    $zeilen = New-Object System.Collections.Generic.List[string]
    if (-not $pyPath) {
        $zeilen.Add('Zielbild nicht zeichenbar: der Python-Starter py.exe fehlt')
        return ,$zeilen
    }
    $ErrorActionPreference = 'Continue'
    foreach ($zeile in @(& $pyPath -3.13 $zielbildPath @Argumente 2>&1)) { $zeilen.Add([string]$zeile) }
    return ,$zeilen
}

function Get-PlanZeilen {
    <# Plan-Ansicht: die Uebersicht aus cockpit.ps1 (eigener pwsh-Prozess, damit ein Fehler
       dort den Tab nicht beendet), darunter die Zeile zum Zielbild. COLUMNS traegt die
       Fensterbreite zum Cockpit wie in der Statuszeile. #>
    param([int]$Breite, [int]$Hoehe)

    $ErrorActionPreference = 'Continue'
    $env:COLUMNS = if ($Breite -gt 0) { [string]$Breite } else { $null }
    $argumente = @('-NoProfile', '-File', $cockpitPath, '-Uebersicht')
    if ($Hoehe -gt 0) { $argumente += @('-Hoehe', [string]$Hoehe) }
    $zeilen = New-Object System.Collections.Generic.List[string]
    foreach ($zeile in @(& $powerShellPath @argumente 2>&1)) {
        $zeilen.Add([string]$zeile)
    }
    if ($LASTEXITCODE -ne 0) { $zeilen.Add("! Übersicht endete mit Exitcode $LASTEXITCODE") }
    $kurz = Invoke-Zielbild @('anzeige', '--kurz')
    $zeilen.Add('')
    $zeilen.Add("Z  Zielbild — was Nakama kann, wenn der Plan fertig ist · $(if ($kurz.Count) { $kurz[0] } else { 'nicht lesbar' })")
    return ,$zeilen
}

function Get-ZielbildDaten {
    <# Zielbild-Ansicht: Kopf (Aktualitaet, steht fest) und Rumpf (wird geblaettert),
       getrennt an der ersten Leerzeile der Ausgabe von `zielbild.py anzeige`. #>
    param([int]$Breite, [switch]$Farbe)

    $argumente = @('anzeige')
    if ($Breite -gt 0) { $argumente += @('--breite', [string]$Breite) }
    if ($Farbe) { $argumente += '--farbe' }
    $kopf = New-Object System.Collections.Generic.List[string]
    $rumpf = New-Object System.Collections.Generic.List[string]
    $imRumpf = $false
    foreach ($zeile in (Invoke-Zielbild $argumente)) {
        if (-not $imRumpf -and $zeile -eq '') { $imRumpf = $true; continue }
        if ($imRumpf) { $rumpf.Add($zeile) } else { $kopf.Add($zeile) }
    }
    return [pscustomobject]@{ Kopf = $kopf; Rumpf = $rumpf }
}

function Get-Seitenhoehe {
    # Rumpfzeilen je Seite: Fensterhoehe ohne Kopf, zwei Leerzeilen, Tastenzeile,
    # Fusszeile, gegebenenfalls die Hinweiszeile zur neuen Fassung und die letzte Zeile,
    # die nie beschrieben wird (sonst rollt das Bild).
    param([object]$Daten, [int]$Hoehe)
    $hinweis = if ($script:FassungsHinweis) { 1 } else { 0 }
    return [math]::Max(3, $Hoehe - $Daten.Kopf.Count - 5 - $hinweis)
}

function Get-ZielbildFrame {
    <# Kopf, eine Seite Rumpf ab $script:Versatz, Tastenzeile. -Hoehe 0 = alles. #>
    param([object]$Daten, [int]$Hoehe)

    $zeilen = New-Object System.Collections.Generic.List[string]
    foreach ($zeile in $Daten.Kopf) { $zeilen.Add($zeile) }
    $zeilen.Add('')
    $rumpf = $Daten.Rumpf
    if ($Hoehe -le 0) {
        foreach ($zeile in $rumpf) { $zeilen.Add($zeile) }
        return ,$zeilen
    }
    $seite = Get-Seitenhoehe $Daten $Hoehe
    $script:Versatz = [math]::Min([math]::Max(0, $script:Versatz), [math]::Max(0, $rumpf.Count - $seite))
    $ende = [math]::Min($rumpf.Count, $script:Versatz + $seite)
    for ($i = $script:Versatz; $i -lt $ende; $i++) { $zeilen.Add($rumpf[$i]) }
    # Auffuellen, damit Tasten- und Fusszeile am unteren Rand stehen bleiben.
    for ($i = $ende - $script:Versatz; $i -lt $seite; $i++) { $zeilen.Add('') }
    $zeilen.Add('')
    $bereich = if ($rumpf.Count) { "Zeilen $($script:Versatz + 1)–$ende von $($rumpf.Count)" } else { 'kein Text' }
    $zeilen.Add("P Plan · ↑↓ Zeile · Bild↑↓ oder Leertaste Seite · Pos1 Ende · Mausrad · $bereich")
    return ,$zeilen
}

function Invoke-Taste {
    <# Eine Taste auf Ansicht und Blaetterstand anwenden. Rueckgabe: 'holen' (andere
       Ansicht, neu rechnen), 'zeichnen' (nur neu zeichnen) oder '' (nichts). #>
    param([ConsoleKeyInfo]$Taste, [int]$Seite, [int]$Gesamt)

    $ziel = switch ($Taste.Key) {
        ([ConsoleKey]::Z) { 'Zielbild' }
        ([ConsoleKey]::P) { 'Plan' }
        ([ConsoleKey]::Tab) { if ($script:Ansicht -eq 'Plan') { 'Zielbild' } else { 'Plan' } }
        default { '' }
    }
    if ($ziel) {
        if ($ziel -eq $script:Ansicht) { return '' }
        $script:Ansicht = $ziel
        return 'holen'
    }
    if ($script:Ansicht -ne 'Zielbild') { return '' }
    $schritt = [math]::Max(1, $Seite - 1)
    $vorher = $script:Versatz
    switch ($Taste.Key) {
        ([ConsoleKey]::UpArrow) { $script:Versatz -= 1 }
        ([ConsoleKey]::DownArrow) { $script:Versatz += 1 }
        ([ConsoleKey]::PageUp) { $script:Versatz -= $schritt }
        ([ConsoleKey]::PageDown) { $script:Versatz += $schritt }
        ([ConsoleKey]::Spacebar) { $script:Versatz += $schritt }
        ([ConsoleKey]::Home) { $script:Versatz = 0 }
        ([ConsoleKey]::End) { $script:Versatz = $Gesamt }
        default { return '' }
    }
    $script:Versatz = [math]::Min([math]::Max(0, $script:Versatz), [math]::Max(0, $Gesamt - $Seite))
    if ($script:Versatz -ne $vorher) { return 'zeichnen' }
    return ''
}

function Read-Taste {
    # Die naechste wartende Taste oder $null; ohne Konsoleneingabe (Umleitung) nie eine.
    try {
        if ([Console]::KeyAvailable) { return [Console]::ReadKey($true) }
    }
    catch { }
    return $null
}

if ($Einmal) {
    try {
        if ($Ansicht -eq 'Zielbild') {
            foreach ($zeile in (Get-ZielbildFrame (Get-ZielbildDaten 0) 0)) { Write-Output $zeile }
        }
        else {
            foreach ($zeile in (Get-PlanZeilen 0 0)) { Write-Output $zeile }
        }
        Write-Output ''
        Write-Output (Get-Fusszeile)
        exit 0
    }
    catch {
        [Console]::Error.WriteLine("Plan-Tab nicht zeichenbar: $($_.Exception.Message)")
        exit 2
    }
}

# NAKAMA_PLAN_TAB_MUTEX lenkt den Mutex fuer Proben um, damit eine Probe neben dem
# laufenden Tab des Users starten kann (wie NAKAMA_DIRIGENT_ANKER im Cockpit).
$mutexName = if ($env:NAKAMA_PLAN_TAB_MUTEX) { $env:NAKAMA_PLAN_TAB_MUTEX } else { 'Local\Nakama-Plan-Tab' }
$mutex = [Threading.Mutex]::new($false, $mutexName)
$besitzt = $false
try { $besitzt = $mutex.WaitOne(0) }
catch [Threading.AbandonedMutexException] { $besitzt = $true }
if (-not $besitzt) {
    $mutex.Dispose()
    Write-Output 'Der Plan-Tab läuft bereits in einem anderen Fenster.'
    exit 0
}

$bindungStart = $null
if ($BindenAn -gt 0) {
    try {
        $gebunden = [Diagnostics.Process]::GetProcessById($BindenAn)
        $bindungStart = $gebunden.StartTime
        $gebunden.Dispose()
    }
    catch {
        # Der gebundene Prozess ist schon beendet: es gibt nichts mehr anzuzeigen.
        $mutex.ReleaseMutex()
        $mutex.Dispose()
        exit 0
    }
}

function Test-Bindung {
    # Lebt der gebundene Prozess noch? Die Startzeit schliesst eine neu vergebene PID aus.
    if ($BindenAn -le 0) { return $true }
    $prozess = $null
    try {
        $prozess = [Diagnostics.Process]::GetProcessById($BindenAn)
        return (-not $prozess.HasExited) -and $prozess.StartTime -eq $bindungStart
    }
    catch {
        return $false
    }
    finally {
        if ($null -ne $prozess) { $prozess.Dispose() }
    }
}

function Get-Fassungsstempel {
    try {
        $datei = [IO.FileInfo]::new($PSCommandPath)
        return "$($datei.LastWriteTimeUtc.Ticks)/$($datei.Length)"
    }
    catch { return '' }
}

function Test-NeueFassung {
    <# $true, wenn diese Datei auf der Platte geaendert ist und fehlerfrei parst. Eine
       fehlerhafte Fassung wird einmal gemeldet und nicht gestartet. #>
    $stempel = Get-Fassungsstempel
    if (-not $stempel -or $stempel -eq $script:Fassung) { return $false }
    $script:Fassung = $stempel
    $token = $null
    $fehler = $null
    try {
        [void][Management.Automation.Language.Parser]::ParseFile($PSCommandPath, [ref]$token, [ref]$fehler)
    }
    catch {
        $script:FassungsHinweis = "Neue Fassung des Plan-Tabs nicht lesbar · die laufende bleibt"
        return $false
    }
    if ($fehler -and $fehler.Count) {
        $script:FassungsHinweis = "Neue Fassung des Plan-Tabs hat $($fehler.Count) Syntaxfehler · die laufende bleibt"
        return $false
    }
    return $true
}

function Write-Frame {
    # Ohne Flackern: Cursor nach oben links, jede Zeile ueberschreiben und ihren Rest
    # loeschen, zuletzt alles darunter. Zeilen werden auf die Fensterbreite gekuerzt,
    # damit kein Umbruch die Hoehe sprengt; eine gekuerzte Zeile verliert ihre Farben.
    param([System.Collections.Generic.List[string]]$Zeilen, [int]$Breite)

    $ausgabe = [Text.StringBuilder]::new()
    [void]$ausgabe.Append("`e[H")
    foreach ($zeile in $Zeilen) {
        $text = $zeile
        if ($Breite -gt 1) {
            $sichtbar = $text -replace $ansiMuster, ''
            if ($sichtbar.Length -ge $Breite) { $text = $sichtbar.Substring(0, $Breite - 2) + '…' }
        }
        [void]$ausgabe.Append($text).Append("`e[0m`e[K").Append("`r`n")
    }
    [void]$ausgabe.Append("`e[J")
    [Console]::Write($ausgabe.ToString())
}

$script:Ansicht = $Ansicht
$script:Versatz = 0
$script:Fassung = Get-Fassungsstempel
$script:FassungsHinweis = ''
$neueFassung = $false
$planZeilen = $null
$zielbild = $null

try { $Host.UI.RawUI.WindowTitle = 'Nakama · Plan' } catch { }
try { [Console]::CursorVisible = $false } catch { }
# Alternativer Bildschirm plus DECSET 1007: das Mausrad kommt als Pfeiltasten an.
[Console]::Write("`e[?1049h`e[?1007h`e[2J")
try {
    $holen = $true
    $naechstes = Get-Date
    :zeichnen while (Test-Bindung) {
        $breite = 0
        $hoehe = 0
        try {
            $breite = [Console]::WindowWidth
            $hoehe = [Console]::WindowHeight
        }
        catch { }
        if ($holen) {
            if (Test-NeueFassung) {
                $neueFassung = $true
                break zeichnen
            }
            try {
                if ($script:Ansicht -eq 'Zielbild') {
                    $zielbild = Get-ZielbildDaten $breite -Farbe
                }
                else {
                    # Sechs Zeilen Reserve: Fehlerzeile, Leerzeile, Zielbildzeile, Hinweis
                    # zur neuen Fassung, Fusszeile, letzte Zeile ohne Rollen.
                    $planZeilen = Get-PlanZeilen $breite ([math]::Max(0, $hoehe - 6))
                }
            }
            catch {
                $fehler = New-Object System.Collections.Generic.List[string]
                $fehler.Add("Plan-Tab: Ansicht nicht lesbar · $($_.Exception.Message)")
                if ($script:Ansicht -eq 'Zielbild') { $zielbild = [pscustomobject]@{ Kopf = $fehler; Rumpf = (New-Object System.Collections.Generic.List[string]) } }
                else { $planZeilen = $fehler }
            }
            $holen = $false
            $naechstes = (Get-Date).AddSeconds($TaktSekunden)
        }
        $bild = New-Object System.Collections.Generic.List[string]
        if ($script:Ansicht -eq 'Zielbild') { $bild.AddRange((Get-ZielbildFrame $zielbild $hoehe)) }
        else { $bild.AddRange($planZeilen) }
        if ($script:FassungsHinweis) { $bild.Add("! $($script:FassungsHinweis)") }
        $bild.Add((Get-Fusszeile))
        Write-Frame $bild $breite

        # Warten bis zum Takt, auf eine Taste oder eine neue Fenstergroesse. Wartende Tasten
        # werden gesammelt verarbeitet und einmal gezeichnet: das Mausrad schickt Serien.
        $runde = 0
        while ($true) {
            if ((Get-Date) -ge $naechstes) { $holen = $true; break }
            Start-Sleep -Milliseconds 100
            $runde++
            if ($runde % 10 -eq 0 -and -not (Test-Bindung)) { break zeichnen }
            try {
                if ([Console]::WindowWidth -ne $breite -or [Console]::WindowHeight -ne $hoehe) { $holen = $true; break }
            }
            catch { }
            $wirkung = ''
            for ($taste = Read-Taste; $null -ne $taste; $taste = Read-Taste) {
                $seite = if ($null -ne $zielbild) { Get-Seitenhoehe $zielbild $hoehe } else { 3 }
                $gesamt = if ($null -ne $zielbild) { $zielbild.Rumpf.Count } else { 0 }
                switch (Invoke-Taste $taste $seite $gesamt) {
                    'holen' { $wirkung = 'holen'; if ($script:Ansicht -eq 'Zielbild') { $script:Versatz = 0 } }
                    'zeichnen' { if (-not $wirkung) { $wirkung = 'zeichnen' } }
                }
            }
            if ($wirkung -eq 'holen') { $holen = $true; break }
            if ($wirkung -eq 'zeichnen') { break }
        }
    }
}
finally {
    [Console]::Write("`e[?1007l`e[?1049l")
    try { [Console]::CursorVisible = $true } catch { }
    try { $mutex.ReleaseMutex() } catch { }
    $mutex.Dispose()
}
if ($neueFassung) {
    # Die neue Fassung laeuft als Kind in derselben Konsole, also im selben Tab; dieser
    # Prozess wartet und endet mit ihr, damit Windows Terminal den Tab nicht schliesst.
    & $powerShellPath -NoLogo -NoProfile -File $PSCommandPath -TaktSekunden $TaktSekunden -BindenAn $BindenAn -Ansicht $script:Ansicht
    exit $LASTEXITCODE
}
exit 0
