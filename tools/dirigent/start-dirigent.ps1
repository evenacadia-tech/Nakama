#requires -Version 7.2

[CmdletBinding()]
param(
    [switch]$InsideTerminal,
    [switch]$PreviewOnly,
    [switch]$ValidateOnly,
    # Nur den Plan-Tab oeffnen, kein Claude (Trockenlauf und Wiederoeffnen, NAK-256).
    [switch]$NurPlanTab,
    # Zielfenster des Plan-Tabs in Windows Terminal: 0 = zuletzt benutztes Fenster,
    # also das eben gestartete Dirigentenfenster. Ein Fenstername dient Proben.
    [ValidatePattern('^[A-Za-z0-9_-]{1,40}\z')]
    [string]$PlanTabFenster = '0',
    # Prozess, mit dessen Ende der Plan-Tab schliesst. 0 = beim Trockenlauf das
    # laufende Dirigentenfenster, falls genau eines laeuft, sonst ungebunden.
    [ValidateRange(0, 2147483647)]
    [int]$PlanTabBindung = 0
)

$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$logoPath = Join-Path $PSScriptRoot 'logo.ps1'
$powerShellPath = (Get-Command pwsh.exe -ErrorAction Stop).Source
$terminalProfile = 'Nakama · Champagne Night'
$terminalSettingsPath = Join-Path $env:LOCALAPPDATA 'Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState\settings.json'
$claudePath = Join-Path $env:USERPROFILE '.local\bin\claude.exe'
# Markerdatei fuer den Neustart in demselben Fenster (siehe -InsideTerminal).
$restartMarker = Join-Path ([IO.Path]::GetTempPath()) 'nakama-dirigent-neustart.marker'
# Plan-Tab (NAK-256): Gesamtplan mit STAND dauerhaft im zweiten Tab desselben
# Fensters (User-Entscheid 11.09.2026: „gesamtplan stand dauerhaft im zweiten tab + statuszeile").
$planTabPath = Join-Path $PSScriptRoot 'plan-tab.ps1'
$planTabTitle = 'Nakama · Plan'
# Prüfhaken: NAKAMA_DIRIGENT_CLAUDE zeigt auf einen Ersatz fuer claude.exe,
# damit sich die Neustartschleife ohne echte Claude-Session messen laesst.
if ($env:NAKAMA_DIRIGENT_CLAUDE) {
    $claudePath = $env:NAKAMA_DIRIGENT_CLAUDE
}

if (-not (Test-Path -LiteralPath $claudePath -PathType Leaf)) {
    $claudeCommand = Get-Command claude.exe -ErrorAction SilentlyContinue
    if ($null -eq $claudeCommand) {
        throw "Claude wurde nicht gefunden: $claudePath"
    }
    $claudePath = $claudeCommand.Source
}

if (-not (Test-Path -LiteralPath $logoPath -PathType Leaf)) {
    throw "Dirigentenlogo wurde nicht gefunden: $logoPath"
}

$claudeArguments = @(
    '--remote-control', 'nakama-dirigent',
    '--model', 'claude-fable-5-1[1m]',
    '--effort', 'xhigh',
    '--permission-mode', 'dontAsk',   # keine Berechtigungsfragen (User-Wort 11.09.2026); allow/deny in .claude/settings.json (Vorlage tools/dirigent/settings.dontask.json)
    '--name', 'nakama-dirigent',
    '/dirigent'
)

function Get-TerminalProfile {
    if (-not (Test-Path -LiteralPath $terminalSettingsPath -PathType Leaf)) {
        return $null
    }

    try {
        $settings = [IO.File]::ReadAllText($terminalSettingsPath, [Text.Encoding]::UTF8) | ConvertFrom-Json -Depth 100
        return @($settings.profiles.list | Where-Object { $_.name -eq $terminalProfile }) | Select-Object -First 1
    }
    catch {
        return $null
    }
}

function Quote-WindowsArgument {
    param([Parameter(Mandatory = $true)][string]$Value)

    if ($Value -notmatch '[\s"]') {
        return $Value
    }

    return '"' + ($Value -replace '(\\*)"', '$1$1\"' -replace '(\\+)$', '$1$1') + '"'
}

function Initialize-TerminalActivation {
    $activationSource = @'
using System;
using System.Runtime.InteropServices;

namespace NakamaDirigentLauncher
{
    [ComImport]
    [Guid("2e941141-7f97-4756-ba1d-9decde894a3d")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IApplicationActivationManager
    {
        [PreserveSig]
        int ActivateApplication(
            [MarshalAs(UnmanagedType.LPWStr)] string appUserModelId,
            [MarshalAs(UnmanagedType.LPWStr)] string arguments,
            uint options,
            out uint processId);

        [PreserveSig]
        int ActivateForFile(
            [MarshalAs(UnmanagedType.LPWStr)] string appUserModelId,
            IntPtr itemArray,
            [MarshalAs(UnmanagedType.LPWStr)] string verb,
            out uint processId);

        [PreserveSig]
        int ActivateForProtocol(
            [MarshalAs(UnmanagedType.LPWStr)] string appUserModelId,
            IntPtr itemArray,
            out uint processId);
    }

    public static class Activation
    {
        public static uint Open(string appUserModelId, string arguments)
        {
            Type type = Type.GetTypeFromCLSID(new Guid("45BA127D-10A8-46EA-8AB7-56EA9078943C"));
            object instance = Activator.CreateInstance(type);
            try
            {
                var manager = (IApplicationActivationManager)instance;
                uint processId;
                int result = manager.ActivateApplication(appUserModelId, arguments, 0, out processId);
                Marshal.ThrowExceptionForHR(result);
                return processId;
            }
            finally
            {
                if (instance != null && Marshal.IsComObject(instance))
                    Marshal.FinalReleaseComObject(instance);
            }
        }
    }
}
'@

    if (-not ('NakamaDirigentLauncher.Activation' -as [type])) {
        Add-Type -TypeDefinition $activationSource
    }
}

function Open-TerminalProfile {
    Initialize-TerminalActivation

    $terminalTitle = if ($PreviewOnly) { 'Nakama · Dirigent Vorschau' } else { 'Nakama Dirigent' }
    $terminalArguments = @(
        '--window', 'new',
        'new-tab',
        '--profile', $terminalProfile,
        '--startingDirectory', $repoRoot,
        '--title', $terminalTitle,
        '--suppressApplicationTitle',
        $powerShellPath,
        '-NoLogo',
        '-NoProfile',
        '-NoExit',
        '-File', $PSCommandPath,
        '-InsideTerminal'
    )
    if ($PreviewOnly) {
        $terminalArguments += '-PreviewOnly'
    }
    $argumentLine = ($terminalArguments | ForEach-Object { Quote-WindowsArgument $_ }) -join ' '

    return [NakamaDirigentLauncher.Activation]::Open(
        'Microsoft.WindowsTerminal_8wekyb3d8bbwe!App',
        $argumentLine
    )
}

function Open-PowerShellFallback {
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $powerShellPath
    $start.UseShellExecute = $true
    foreach ($argument in @('-NoLogo', '-NoProfile', '-NoExit', '-File', $PSCommandPath, '-InsideTerminal')) {
        [void]$start.ArgumentList.Add($argument)
    }
    if ($PreviewOnly) {
        [void]$start.ArgumentList.Add('-PreviewOnly')
    }
    return [Diagnostics.Process]::Start($start).Id
}

function Get-PlanTabProcesses {
    # Laufende Plan-Tabs (Schleife, nicht -Einmal), erkannt am Befehlstext: pwsh
    # mit `-File ...\plan-tab.ps1`. Ein Prozess, der den Namen nur im Text eines
    # -Command traegt, zaehlt nicht. Ist die Prozessliste nicht lesbar, gilt keiner
    # als laufend; einen zweiten Start faengt plan-tab.ps1 selbst ueber seinen Mutex ab.
    try {
        return @(Get-CimInstance Win32_Process -Filter "Name = 'pwsh.exe'" -ErrorAction Stop | Where-Object {
            $_.CommandLine -match '(?i)^(?:"[^"]*"|\S+)\s+(?:-\S+\s+)*-File\s+(?:"[^"]*[\\/]plan-tab\.ps1"|[^"\s]*[\\/]plan-tab\.ps1)(?:\s|$)' -and
                $_.CommandLine -notmatch '(?i)\s-Einmal(?:\s|$)'
        })
    }
    catch {
        return @()
    }
}

function Find-DirigentStarterPid {
    # Das laufende Dirigentenfenster: genau ein Starter mit -InsideTerminal ohne
    # -PreviewOnly. Sonst 0, der Plan-Tab bleibt dann ungebunden.
    try {
        $starter = @(Get-CimInstance Win32_Process -Filter "Name = 'pwsh.exe'" -ErrorAction Stop | Where-Object {
            $_.ProcessId -ne $PID -and
                $_.CommandLine -match '(?i)^(?:"[^"]*"|\S+)\s+(?:-\S+\s+)*-File\s+(?:"[^"]*[\\/]start-dirigent\.ps1"|[^"\s]*[\\/]start-dirigent\.ps1)\s' -and
                $_.CommandLine -match '(?i)\s-InsideTerminal(?:\s|$)' -and
                $_.CommandLine -notmatch '(?i)\s-PreviewOnly(?:\s|$)'
        })
        if ($starter.Count -eq 1) { return [int]$starter[0].ProcessId }
    }
    catch { }
    return 0
}

function Open-PlanTab {
    <# Oeffnet den Plan-Tab, wenn keiner laeuft: mit Windows Terminal als Tab im
       Fenster -PlanTabFenster und danach zurueck auf den ersten Tab (der Dirigent
       bleibt vorn, der Plan liegt im zweiten Tab); ohne Windows Terminal als
       eigenes Fenster. Wartet, bis der Prozess sichtbar ist. Wirft nie: der
       Dirigent startet auch ohne Plan-Tab. #>
    param([int]$Bindung)

    $ergebnis = [pscustomobject]@{ Status = 'Fehler'; Pid = 0; Weg = ''; Hinweis = '' }
    try {
        $laufend = @(Get-PlanTabProcesses)
        if ($laufend.Count) {
            $ergebnis.Status = 'läuft schon'
            $ergebnis.Pid = [int]$laufend[0].ProcessId
            return $ergebnis
        }
        if (-not (Test-Path -LiteralPath $planTabPath -PathType Leaf)) {
            $ergebnis.Status = 'fehlt'
            $ergebnis.Hinweis = $planTabPath
            return $ergebnis
        }
        $tabArguments = @('-NoLogo', '-NoProfile', '-File', $planTabPath)
        if ($Bindung -gt 0) {
            $tabArguments += @('-BindenAn', [string]$Bindung)
        }

        if ($null -ne $profile) {
            try {
                Initialize-TerminalActivation
                $terminalArguments = @(
                    '--window', $PlanTabFenster,
                    'new-tab',
                    '--profile', $terminalProfile,
                    '--startingDirectory', $repoRoot,
                    '--title', $planTabTitle,
                    '--suppressApplicationTitle',
                    $powerShellPath
                ) + $tabArguments + @(';', 'focus-tab', '--target', '0')
                $argumentLine = ($terminalArguments | ForEach-Object { Quote-WindowsArgument $_ }) -join ' '
                [void][NakamaDirigentLauncher.Activation]::Open('Microsoft.WindowsTerminal_8wekyb3d8bbwe!App', $argumentLine)
                $ergebnis.Weg = 'Windows Terminal'
            }
            catch {
                $ergebnis.Hinweis = "Windows Terminal: $($_.Exception.Message)"
            }
        }
        if (-not $ergebnis.Weg) {
            $start = [Diagnostics.ProcessStartInfo]::new()
            $start.FileName = $powerShellPath
            $start.UseShellExecute = $true
            foreach ($argument in $tabArguments) {
                [void]$start.ArgumentList.Add($argument)
            }
            $prozess = [Diagnostics.Process]::Start($start)
            if ($null -ne $prozess) { $prozess.Dispose() }
            $ergebnis.Weg = 'eigenes Fenster'
        }

        $frist = (Get-Date).AddSeconds(8)
        while ((Get-Date) -lt $frist) {
            $gesehen = @(Get-PlanTabProcesses)
            if ($gesehen.Count) {
                $ergebnis.Status = 'geöffnet'
                $ergebnis.Pid = [int]$gesehen[0].ProcessId
                return $ergebnis
            }
            Start-Sleep -Milliseconds 250
        }
        $ergebnis.Status = 'nicht sichtbar'
        return $ergebnis
    }
    catch {
        $ergebnis.Status = 'Fehler'
        $ergebnis.Hinweis = $_.Exception.Message
        return $ergebnis
    }
}

function Format-PlanTabMeldung {
    param([object]$Ergebnis)

    switch ($Ergebnis.Status) {
        'geöffnet' {
            if ($Ergebnis.Weg -eq 'eigenes Fenster') { return 'Gesamtplan in einem eigenen Fenster geöffnet.' }
            return 'Gesamtplan im zweiten Tab geöffnet.'
        }
        'läuft schon' { return 'Gesamtplan läuft bereits.' }
        'fehlt' { return "Gesamtplan nicht geöffnet: $($Ergebnis.Hinweis) fehlt." }
        'nicht sichtbar' { return 'Gesamtplan angefordert, aber nach 8 Sekunden nicht sichtbar.' }
        default { return "Gesamtplan nicht geöffnet: $($Ergebnis.Hinweis)" }
    }
}

$profile = Get-TerminalProfile

if ($NurPlanTab) {
    # Die Meldung wird meist mitgeschnitten (Werkzeugaufruf des Dirigenten): UTF-8 statt Konsolen-Codepage.
    [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
    $bindung = if ($PlanTabBindung -gt 0) { $PlanTabBindung } else { Find-DirigentStarterPid }
    $planTab = Open-PlanTab -Bindung $bindung
    $gebunden = if ($bindung -gt 0) { "gebunden an PID $bindung" } else { 'ungebunden' }
    $weg = if ($planTab.Weg) { $planTab.Weg } else { '—' }
    [Console]::WriteLine("PLAN-TAB · $($planTab.Status) · PID $($planTab.Pid) · $weg · $gebunden")
    if ($planTab.Hinweis) {
        [Console]::WriteLine("HINWEIS · $($planTab.Hinweis)")
    }
    if ($planTab.Status -notin @('geöffnet', 'läuft schon')) {
        exit 1
    }
    return
}

if ($ValidateOnly) {
    $validation = [ordered]@{
        RepoRoot = $repoRoot
        LogoPath = $logoPath
        ClaudePath = $claudePath
        PowerShellPath = $powerShellPath
        TerminalProfile = $terminalProfile
        TerminalProfileAvailable = ($null -ne $profile)
        TerminalFont = if ($null -ne $profile) { [string]$profile.font.face } else { '' }
        TerminalColorScheme = if ($null -ne $profile) { [string]$profile.colorScheme } else { '' }
        ClaudeArguments = $claudeArguments
        RestartMarker = $restartMarker
        RemoteControl = $true
        PreviewStartsClaude = $false
        FallbackAvailable = $true
    }
    $validation | ConvertTo-Json -Depth 10
    return
}

if ($InsideTerminal) {
    Set-Location -LiteralPath $repoRoot
    $Host.UI.RawUI.WindowTitle = if ($PreviewOnly) { 'Nakama · Dirigent Vorschau' } else { 'Nakama Dirigent' }
    & $logoPath
    [Console]::WriteLine('')
    if ($PreviewOnly) {
        [Console]::WriteLine('Lokale Vorschau — Claude wurde nicht gestartet.')
        return
    }

    # Neustart in demselben Fenster (01.09.2026). Vorher startete der Dirigent
    # fuer eine frische Session den Starter abgekoppelt (neues Terminalfenster)
    # und beendete danach seine eigene PID: das alte Fenster blieb mit einer
    # roten Fehlermeldung stehen, und der Neustart hing an der COM-Aktivierung
    # des Terminals. Jetzt legt der Dirigent nur die Markerdatei an und beendet
    # sich (Skill §5); diese Schleife startet sofort eine frische Session hier.
    # Ohne Marker (Absturz, /exit) wartet das Fenster auf eine Taste, statt
    # blind neu zu starten - eine kaputte Konfiguration soll nicht in einer
    # Endlosschleife kreisen.
    $runde = 0
    while ($true) {
        $runde++
        if (Test-Path -LiteralPath $restartMarker) { [IO.File]::Delete($restartMarker) }
        # Plan-Tab (NAK-256): bei jedem Start genau einer. Der Marker-Neustart findet
        # den laufenden Tab und oeffnet keinen zweiten; an diesen Prozess gebunden,
        # schliesst der Tab mit dem Dirigentenfenster.
        [Console]::WriteLine((Format-PlanTabMeldung (Open-PlanTab -Bindung $PID)))
        $beginn = Get-Date
        & $claudePath @claudeArguments
        $exit = $LASTEXITCODE
        $minuten = [int]((Get-Date) - $beginn).TotalMinutes
        if (Test-Path -LiteralPath $restartMarker) {
            [IO.File]::Delete($restartMarker)
            [Console]::WriteLine('')
            [Console]::WriteLine("Neustart angefordert · Session $runde lief $minuten min · frische Dirigenten-Session in diesem Fenster ...")
            Start-Sleep -Seconds 2
            [Console]::WriteLine('')
            & $logoPath
            [Console]::WriteLine('')
            continue
        }
        [Console]::WriteLine('')
        if ($exit -ne 0) {
            [Console]::WriteLine("Claude wurde mit Exitcode $exit beendet · Session $runde lief $minuten min.")
        }
        else {
            [Console]::WriteLine("Dirigent beendet · Session $runde lief $minuten min.")
        }
        [Console]::WriteLine('Enter = neue Dirigenten-Session in diesem Fenster · Esc = Fenster schliessen')
        $taste = $null
        try { $taste = [Console]::ReadKey($true) } catch { }
        if ($null -eq $taste -or $taste.Key -eq [ConsoleKey]::Escape) { [Environment]::Exit(0) }
        [Console]::WriteLine('')
    }
}

if ($null -ne $profile) {
    try {
        [void](Open-TerminalProfile)
        return
    }
    catch {
        # Das lokale Terminal bleibt der sichere Ersatzweg, falls die App-Aktivierung ausfällt.
    }
}

[void](Open-PowerShellFallback)
