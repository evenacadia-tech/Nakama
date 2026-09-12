<#
.SYNOPSIS
    Registriert die drei erhoehten Aufgaben \Nakama\installieren, \Nakama\pruefen
    und \Nakama\rueckweg, ueber die der Dirigent Install-Nakama.ps1 ohne
    UAC-Prompt faehrt (User-Entscheid 12.09.2026, Register NAK-285).

.DESCRIPTION
    Einmal je Rechner mit Rechteerhoehung ausfuehren. Danach startet eine NICHT
    erhoehte Session die Aufgabe:

        Start-ScheduledTask -TaskPath '\Nakama\' -TaskName installieren
        (Get-ScheduledTaskInfo -TaskPath '\Nakama\' -TaskName installieren).LastTaskResult

    Die Aufgabe laeuft als derselbe Benutzer mit hoechsten Rechten, nur bei
    angemeldetem Benutzer, ohne gespeichertes Kennwort. Program Files bleibt fuer
    jeden anderen Benutzerprozess geschuetzt (Manifest ziele.broker_verzeichnis_warum);
    die Riegel des Installers - Manifest-Hash, Ruhe bei laufendem FL, Rueckweg mit
    NAK-41-Riegel - laufen unveraendert. -Rueckweg -Erzwingen bleibt bewusst ohne
    Aufgabe: das ist ein User-Entscheid.

    Ausgabe je Lauf: %LOCALAPPDATA%\evenacadia\nakama-installer\<modus>.log
    (gesamte Installerausgabe, wird je Lauf ueberschrieben). Exitcode der Aufgabe
    = Exitcode des Installers.

.PARAMETER Repo
    Repo-Wurzel, aus der installiert wird (Standard: der sichtbare Checkout).

.PARAMETER Entfernen
    Die drei Aufgaben wieder austragen (Rueckweg dieses Skripts).
#>
[CmdletBinding()]
param(
    [string]$Repo = 'C:\Users\phili\Projekte\Nakama',
    [switch]$Entfernen
)

$ErrorActionPreference = 'Stop'

$istErhoeht = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $istErhoeht) {
    Write-Host 'ABBRUCH: braucht Rechteerhoehung - Aufgaben mit hoechsten Rechten registriert nur ein erhoehter Prozess.' -ForegroundColor Red
    exit 3
}

$aufgabenPfad = '\Nakama\'
$modi = [ordered]@{ installieren = ''; pruefen = ' -Pruefen'; rueckweg = ' -Rueckweg' }

if ($Entfernen) {
    foreach ($m in $modi.Keys) {
        Unregister-ScheduledTask -TaskPath $aufgabenPfad -TaskName $m -Confirm:$false -ErrorAction SilentlyContinue
    }
    Write-Host '[OK] Aufgaben \Nakama\* ausgetragen.'
    exit 0
}

$installer = Join-Path $Repo 'eq-copilot\install\Install-Nakama.ps1'
if (-not (Test-Path -LiteralPath $installer)) {
    Write-Host "ABBRUCH: Installer fehlt: $installer" -ForegroundColor Red
    exit 1
}
$pwsh = (Get-Command pwsh.exe).Source
$logOrdner = Join-Path $env:LOCALAPPDATA 'evenacadia\nakama-installer'
New-Item -ItemType Directory -Force -Path $logOrdner | Out-Null

$principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Highest
$settings  = New-ScheduledTaskSettingsSet -ExecutionTimeLimit (New-TimeSpan -Minutes 15) `
    -MultipleInstances IgnoreNew -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries

foreach ($m in $modi.Keys) {
    $log    = Join-Path $logOrdner "$m.log"
    # Einfache Anfuehrungszeichen um die Pfade, doppelte nur um -Command: die
    # Argumentzerlegung von pwsh.exe laesst so alles heil (CLAUDE.md, Landminen).
    $befehl = "& '$installer'$($modi[$m]) *> '$log'; exit `$LASTEXITCODE"
    $arg    = "-NoProfile -ExecutionPolicy Bypass -Command `"$befehl`""
    $action = New-ScheduledTaskAction -Execute $pwsh -Argument $arg -WorkingDirectory $Repo
    Register-ScheduledTask -TaskPath $aufgabenPfad -TaskName $m -Action $action `
        -Principal $principal -Settings $settings -Force | Out-Null
    Write-Host "[OK] \Nakama\$m  ->  pwsh $arg"
}

Write-Host ''
Get-ScheduledTask -TaskPath $aufgabenPfad | Format-Table TaskName, State -AutoSize
Write-Host "Log-Ordner: $logOrdner"
