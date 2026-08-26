#requires -Version 7.2

[CmdletBinding()]
param(
    [switch]$InsideTerminal,
    [switch]$PreviewOnly,
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$logoPath = Join-Path $PSScriptRoot 'logo.ps1'
$powerShellPath = (Get-Command pwsh.exe -ErrorAction Stop).Source
$terminalProfile = 'Nakama · Champagne Night'
$terminalSettingsPath = Join-Path $env:LOCALAPPDATA 'Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState\settings.json'
$claudePath = Join-Path $env:USERPROFILE '.local\bin\claude.exe'

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
    '--model', 'fable',
    '--effort', 'xhigh',
    '--permission-mode', 'auto',
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

function Open-TerminalProfile {
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

$profile = Get-TerminalProfile

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
        RemoteControl = $false
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
    & $claudePath @claudeArguments
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Claude wurde mit Exitcode $LASTEXITCODE beendet."
    }
    return
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
