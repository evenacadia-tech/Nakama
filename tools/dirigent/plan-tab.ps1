#requires -Version 7.2
<#
Gesamtplan dauerhaft im zweiten Terminal-Tab (NAK-256). User-Entscheid 11.09.2026:
„gesamtplan stand dauerhaft im zweiten tab + statuszeile".

Zeichnet im Takt von -TaktSekunden den Bildschirm neu: oben STAND, darunter der
Gesamtplan (beides aus `cockpit.ps1 -Uebersicht`, bei zu kleinem Fenster mit
gefalteten fertigen Phasen), unten die Uhrzeit des Zeichnens sowie Kontext und
Kontingente aus dem Telemetrie-Cache der Dirigenten-Statuszeile. Rein lesend:
der Tab schreibt weder eine Projektdatei noch einen Cache.

-Einmal    zeichnet genau einmal, ohne Bildschirmsteuerung (Probe, Umleitung).
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
    [int]$BindenAn = 0
)

$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8

$cockpitPath = Join-Path $PSScriptRoot 'cockpit.ps1'
$powerShellPath = (Get-Process -Id $PID).Path

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

function Get-FrameZeilen {
    <# Ein Bild: die Uebersicht aus cockpit.ps1 (eigener pwsh-Prozess, damit ein
       Fehler dort den Tab nicht beendet), darunter die Fusszeile. COLUMNS traegt
       die Fensterbreite zum Cockpit wie in der Statuszeile. #>
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
    $zeilen.Add('')
    $zeilen.Add("aktualisiert $((Get-Date).ToString('HH:mm:ss')) · $(Get-TelemetrieText)")
    return ,$zeilen
}

if ($Einmal) {
    try {
        foreach ($zeile in (Get-FrameZeilen 0 0)) { Write-Output $zeile }
        exit 0
    }
    catch {
        [Console]::Error.WriteLine("Plan-Tab nicht zeichenbar: $($_.Exception.Message)")
        exit 2
    }
}

$mutex = [Threading.Mutex]::new($false, 'Local\Nakama-Plan-Tab')
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

function Write-Frame {
    # Ohne Flackern: Cursor nach oben links, jede Zeile ueberschreiben und ihren
    # Rest loeschen, zuletzt alles darunter. Zeilen werden auf die Fensterbreite
    # gekuerzt, damit kein Umbruch die Hoehe sprengt.
    param([System.Collections.Generic.List[string]]$Zeilen, [int]$Breite)

    $ausgabe = [Text.StringBuilder]::new()
    [void]$ausgabe.Append("`e[H")
    foreach ($zeile in $Zeilen) {
        $text = $zeile
        if ($Breite -gt 1 -and $text.Length -ge $Breite) { $text = $text.Substring(0, $Breite - 2) + '…' }
        [void]$ausgabe.Append($text).Append("`e[K").Append("`r`n")
    }
    [void]$ausgabe.Append("`e[J")
    [Console]::Write($ausgabe.ToString())
}

try { $Host.UI.RawUI.WindowTitle = 'Nakama · Plan' } catch { }
try { [Console]::CursorVisible = $false } catch { }
[Console]::Write("`e[2J")
try {
    while (Test-Bindung) {
        $breite = 0
        $hoehe = 0
        try {
            $breite = [Console]::WindowWidth
            $hoehe = [Console]::WindowHeight
        }
        catch { }
        try {
            # Vier Zeilen Reserve: Fehlerzeile, Leerzeile, Fusszeile, letzte Zeile ohne Rollen.
            $zeilen = Get-FrameZeilen $breite ([math]::Max(0, $hoehe - 4))
        }
        catch {
            $zeilen = New-Object System.Collections.Generic.List[string]
            $zeilen.Add("Plan-Tab: Übersicht nicht lesbar · $($_.Exception.Message)")
        }
        Write-Frame $zeilen $breite
        $naechstes = (Get-Date).AddSeconds($TaktSekunden)
        while ((Get-Date) -lt $naechstes) {
            Start-Sleep -Milliseconds 1000
            if (-not (Test-Bindung)) { break }
            try {
                if ([Console]::WindowWidth -ne $breite -or [Console]::WindowHeight -ne $hoehe) { break }
            }
            catch { }
        }
    }
}
finally {
    try { [Console]::CursorVisible = $true } catch { }
    try { $mutex.ReleaseMutex() } catch { }
    $mutex.Dispose()
}
exit 0
