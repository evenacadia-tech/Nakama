#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1: faehrt einzelne Kanon-Beine mit genau dem Befehl des
    Runners (tools/beweise.ps1: Pruefbinaer eq-copilot\build\plugin\<Name>_artefacts\Release\<Name>.exe
    bzw. py -3.13 tools\eq-copilot\<Name>, Arbeitsverzeichnis = Wurzel) und
    haengt je Bein eine Zeile an die Exitcode-Datei.

.PARAMETER Kuerzel
    Beinkuerzel wie im Runner (A1, B6, B10 ...).

.PARAMETER Ziel
    Exitcode-Datei (Standard docs/beweise/roh/NAK-289-etappe1-beine.txt).

.PARAMETER Titel
    Anlass der Zeilen (z. B. "a) exception-escape").

.PARAMETER Roh
    Optional: vollstaendige stdout/stderr aller gefahrenen Beine in diese Datei.

.NOTES
    Exit 0 = alle gefahrenen Beine Exit 0; 2 = mindestens eines nicht.
    Nur Werkzeug des Erbauers; misst nichts, was der Kanon nicht misst.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string[]] $Kuerzel,
    [string] $Ziel = 'docs/beweise/roh/NAK-289-etappe1-beine.txt',
    [string] $Titel = '',
    [string] $Roh = '',
    [int] $ZeitlimitMinuten = 30
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if (-not (Test-Path -LiteralPath (Join-Path $Wurzel 'tools\beweise.ps1'))) {
    $Wurzel = 'C:\Users\phili\Projekte\Nakama'
}

# Name, Art und Argumente je Kuerzel, abgeschrieben aus tools/beweise.ps1 (Stand 432f209c).
$beine = @{
    'A1'  = @{ Name = 'EqCopNullTest';                       Art = 'plugin'; Argumente = @() }
    'A2'  = @{ Name = 'EqCopGoldenTest';                     Art = 'plugin'; Argumente = @((Join-Path $Wurzel 'eq-copilot\fixtures')) }
    'A3'  = @{ Name = 'EqCopMarkierungTest';                 Art = 'plugin'; Argumente = @() }
    'A4b' = @{ Name = 'EqCopPipeClientTest';                 Art = 'plugin'; Argumente = @() }
    'A16' = @{ Name = 'EqCopProbeeqNullTest';                Art = 'plugin'; Argumente = @() }
    'A22' = @{ Name = 'pruefe_ipc_last.py';                  Art = 'python'; Argumente = @() }
    'B2'  = @{ Name = 'EqCopStateMigrationTest';             Art = 'plugin'; Argumente = @() }
    'B3b' = @{ Name = 'EqCopHostProbeTest';                  Art = 'plugin'; Argumente = @() }
    'B3c' = @{ Name = 'EqCopSchemaTest';                     Art = 'plugin'; Argumente = @() }
    'B4'  = @{ Name = 'EqCopQueueStressTest';                Art = 'plugin'; Argumente = @() }
    'B5'  = @{ Name = 'EqCopAnalysisGoldenTest';             Art = 'plugin'; Argumente = @() }
    'B6'  = @{ Name = 'EqCopDspGoldenTest';                  Art = 'plugin'; Argumente = @() }
    'B7'  = @{ Name = 'EqCopTransactionTest';                Art = 'plugin'; Argumente = @() }
    'B10' = @{ Name = 'EqCopIpcTest';                        Art = 'plugin'; Argumente = @() }
    'B13' = @{ Name = 'EqCopSonde012SourcesModelTest';       Art = 'plugin'; Argumente = @() }
    'B16' = @{ Name = 'EqCopSonde013EventWireTest';          Art = 'plugin'; Argumente = @() }
    'B17' = @{ Name = 'EqCopSonde013TruePeakGoldenTest';     Art = 'plugin'; Argumente = @() }
    'B18' = @{ Name = 'EqCopSonde013DynamicsTest';           Art = 'plugin'; Argumente = @() }
    'B19' = @{ Name = 'EqCopSonde013StereoGoldenTest';       Art = 'plugin'; Argumente = @() }
}

function Fahre-Bein {
    param([string] $K)
    $b = $beine[$K]
    if ($null -eq $b) { throw "Unbekanntes Kuerzel $K" }
    if ($b.Art -eq 'plugin') {
        $datei = Join-Path $Wurzel ("eq-copilot\build\plugin\{0}_artefacts\Release\{0}.exe" -f $b.Name)
        $argumente = @($b.Argumente)
        $befehl = ("eq-copilot\build\plugin\{0}_artefacts\Release\{0}.exe" -f $b.Name)
    }
    else {
        $datei = 'py'
        $argumente = @('-3.13', (Join-Path $Wurzel ('tools\eq-copilot\' + $b.Name))) + @($b.Argumente)
        $befehl = 'py -3.13 tools\eq-copilot\' + $b.Name
    }
    $psi = [System.Diagnostics.ProcessStartInfo]::new($datei)
    foreach ($a in $argumente) { $psi.ArgumentList.Add([string] $a) }
    $psi.WorkingDirectory = $Wurzel
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = [Text.Encoding]::UTF8
    $psi.StandardErrorEncoding = [Text.Encoding]::UTF8
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $p = [System.Diagnostics.Process]::Start($psi)
    $aus = $p.StandardOutput.ReadToEndAsync()
    $err = $p.StandardError.ReadToEndAsync()
    $code = $null
    if (-not $p.WaitForExit($ZeitlimitMinuten * 60 * 1000)) {
        try { $p.Kill($true) } catch { }
        $code = 124
    }
    $p.WaitForExit()
    if ($null -eq $code) { $code = $p.ExitCode }
    $uhr.Stop()
    $stdout = $aus.Result
    $stderr = $err.Result
    $letzte = @(($stdout -split "`r?`n") | Where-Object { $_.Trim() }) | Select-Object -Last 1
    [pscustomobject]@{
        Kuerzel  = $K
        Name     = $b.Name
        Befehl   = $befehl
        ExitCode = $code
        Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 2)
        Letzte   = [string] $letzte
        StdOut   = $stdout
        StdErr   = $stderr
    }
}

$zielPfad = if ([IO.Path]::IsPathRooted($Ziel)) { $Ziel } else { Join-Path $Wurzel $Ziel }
$sha = (git -C $Wurzel rev-parse --short=8 HEAD).Trim()
$schmutz = @(git -C $Wurzel status --porcelain --untracked-files=no).Count
$rot = 0
foreach ($k in $Kuerzel) {
    $r = Fahre-Bein $k
    if ($r.ExitCode -ne 0) { $rot++ }
    $zeile = '{0} | HEAD {1}{2} | {3} | {4} {5} | Exit {6} | {7} s | {8}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), $sha,
        $(if ($schmutz -gt 0) { " (+$schmutz geaenderte Datei(en))" } else { '' }), $Titel, $r.Kuerzel, $r.Name, $r.ExitCode,
        ($r.Sekunden.ToString([Globalization.CultureInfo]::InvariantCulture)), ($r.Letzte -replace '\s+', ' ')
    Add-Content -LiteralPath $zielPfad -Value $zeile -Encoding utf8NoBOM
    Write-Host $zeile
    if ($Roh) {
        $rohPfad = if ([IO.Path]::IsPathRooted($Roh)) { $Roh } else { Join-Path $Wurzel $Roh }
        Add-Content -LiteralPath $rohPfad -Value @(
            "===== $($r.Kuerzel) $($r.Name) | $($r.Befehl) | Exit $($r.ExitCode) | $($r.Sekunden) s",
            '----- stdout', $r.StdOut.TrimEnd(), '----- stderr', $r.StdErr.TrimEnd(), '') -Encoding utf8NoBOM
    }
}
exit $(if ($rot -eq 0) { 0 } else { 2 })
