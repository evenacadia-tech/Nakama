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
         befuellen (Diagnoseprojekt plus Samples\Testtrack.wav, dazu die
         Referenzprojekte aus Karte U43 unter den Namen der Szenariodatei;
         SHA-256 am Anfang und am Ende), Briefkasten-Ordner aufraeumen
         (zuerst die Anfrage, dann die Antworten), vor dem FL-Start die
         Renders ueber FL64.exe /R aus der Arbeitskopie, Folge im Protokoll -
         fuer nulltest-host drei: das Diagnoseprojekt nach render\, danach
         jedes der zwei Referenzprojekte nach render\referenz\<Name>\ mit
         eigenem render.json (P-21; ein fehlendes bekommt dort einen
         Renderstatus mit Grund und keinen Render) -, FL mit dem Projekt
         starten, auf den Ping des Controller-Skripts warten.
      4. Szenarien: jede JSON-Datei unter docs/gesundheit/szenarien/ alphabetisch
         ueber tools/fl/szenario.py; `frischer_start` startet das Diagnose-FL
         vorher neu. Szenario-Exit 3 bricht die Folge ab, Szenario-Exit 5 laesst
         sie weiterlaufen. Nach ABWEICHUNG im Nulltest faehrt der Rueckweg, die
         Folgeszenarien entfallen mit Meldung.
      5. Kopfzeile "LAUFZEIT <Ticket> <sha> <Urteil> ..." auf stdout - sie
         gehoert ins Manifest. Jeder Ausgang raeumt anfrage.json ab.

    Exit 0 = gemessen oder begruendet uebersprungen, 3 = Voraussetzung fehlt
    (Hashes, Installation, Port, Ping; ein Szenario mit Exit 3 oder 5),
    4 = mindestens ein Szenario verfehlt. Diagnose- und Referenzprojekte
    werden nie gespeichert; FL bleibt danach offen (-Beenden schliesst es, z. B. im
    naechtlichen Lauf). Der Runner startet nie den Broker.

    -Selbsttest faehrt die Faelle der Matrixzeilen gegen Attrappen fuer
    FL-Instanzen, Prozessliste, Aufgaben, Uhr, py, git, FL64 und Dateien - ohne
    FL und ohne Installation; Exit 0 gruen, 4 rot, 2 Werkzeugfehler. -Nur <fall>
    faehrt einzelne Faelle.

.EXAMPLE
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket NAK-286 -Basis 467e4534
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket PROBE -Erzwingen -OhneInstallation -Neustart
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Selbsttest
#>
[CmdletBinding()]
param(
    [string]$Ticket,
    [string]$Basis,
    [switch]$Erzwingen,
    [switch]$OhneInstallation,
    [switch]$Neustart,
    [switch]$Beenden,
    [switch]$Selbsttest,
    [string[]]$Nur,
    [string]$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [string]$McpRepo = 'C:\Users\phili\Projekte\fl-studio-mcp',
    [string]$Projekt,
    [string]$Sample,
    [string]$Roh,
    [string[]]$Szenarien,
    [string]$LoopMidi = 'C:\Program Files (x86)\Tobias Erichsen\loopMIDI\loopMIDI.exe',
    [int]$PingFristSekunden = 150,
    [int]$InstallFristSekunden = 900,
    [int]$RenderFristSekunden = 600
)

$ErrorActionPreference = 'Stop'
$env:PYTHONIOENCODING = 'utf-8'

$script:Produktpfade = @('eq-copilot/plugin/', 'eq-copilot/schemas/', 'broker/src/', 'eq-copilot/install/')
$script:AntwortMuster = '^[0-9a-f]{32}\.(gen|probeeq|broker)\.[0-9]+\.[0-9a-f]{16}\.[0-9a-f]{32}\.json$'
$script:TempMuster = '^[0-9a-f]{32}\.(gen|probeeq|broker)\.[0-9]+\.[0-9a-f]{16}\.[0-9a-f]{32}\.json\.tmp-[0-9]+$'
$script:EndeMarke = '__LAUFZEIT_ENDE__'
$script:ReferenzFehlt = 'Referenzprojekt fehlt (Karte U43, K-286-1)'
$script:K = $null
$script:U = $null

# ---------------------------------------------------------------- Kontext, Protokoll, Ausgang
function Neuer-Kontext([hashtable]$w) {
    $arbeit = $w.Arbeit
    $projekt = $w.Projekt
    if (-not $projekt) { $projekt = Join-Path $w.Repo 'eq-copilot\fixtures\fl\Nakama-Diagnose.flp' }
    return @{
        Ticket = $w.Ticket; Basis = $w.Basis; Head = $w.Head; Repo = $w.Repo; McpRepo = $w.McpRepo
        Erzwingen = [bool]$w.Erzwingen; OhneInstallation = [bool]$w.OhneInstallation
        Neustart = [bool]$w.Neustart; Beenden = [bool]$w.Beenden
        Projekt = $projekt; ProjektName = [IO.Path]::GetFileName($projekt); Sample = $w.Sample
        Roh = $w.Roh; Szenarien = $w.Szenarien; LoopMidi = $w.LoopMidi
        PingFristSekunden = $w.PingFristSekunden; InstallFristSekunden = $w.InstallFristSekunden
        RenderFristSekunden = $w.RenderFristSekunden
        Arbeit = $arbeit; ProjektOrdner = (Join-Path $arbeit 'projekt'); RenderOrdner = (Join-Path $arbeit 'render')
        LogDatei = (Join-Path $arbeit 'laufzeit.log')
        DiagnoseOrdner = $w.DiagnoseOrdner; AntwortOrdner = (Join-Path $w.DiagnoseOrdner 'antwort')
        SzenarioPy = (Join-Path $w.Repo 'tools\fl\szenario.py')
        Leise = [bool]$w.Leise
        Protokoll = (New-Object System.Collections.Generic.List[string])
        Kopfzeilen = (New-Object System.Collections.Generic.List[string])
        FlStart = $null; DiagnosePid = $null; Ping = $null; Installation = ''
        ProjektHashes = $null; ReferenzHashes = [ordered]@{}; ControllerNeu = $false; ScriptVersion = $null
        Ergebnis = $null; EndeLaeuft = $false
    }
}

function Zeit { (& $script:U.Jetzt).ToString('yyyy-MM-dd HH:mm:ss') }

function Log([string]$text) {
    $zeile = "[{0}] {1}" -f (Zeit), $text
    if (-not $script:K.Leise) { Write-Host $zeile }
    try { Add-Content -LiteralPath $script:K.LogDatei -Value $zeile -Encoding utf8 } catch { }
    $script:K.Protokoll.Add($zeile)
}

function Schreibe-Roh([string]$urteil, [string]$zusatz) {
    $K = $script:K
    $kopf = @(
        "# Laufzeit-Arm $($K.Ticket) - $urteil",
        "",
        "Zeit: $(Zeit) | Basis: $($K.Basis) | HEAD: $($K.Head) | Repo: $($K.Repo) | Projekt: $($K.ProjektName)",
        "Kopfzeile: LAUFZEIT $($K.Ticket) $($K.Head) $urteil $zusatz",
        "",
        "## Protokoll",
        "",
        '```text'
    ) + @($K.Protokoll) + @('```', '')
    New-Item -ItemType Directory -Force -Path (Split-Path $K.Roh) | Out-Null
    $szenarienTeil = ''
    if (Test-Path -LiteralPath $K.Roh) {
        $alt = Get-Content -LiteralPath $K.Roh -Raw -Encoding utf8
        if ($alt) {
            $idx = ([string]$alt).IndexOf('## Szenarien')
            if ($idx -ge 0) { $szenarienTeil = ([string]$alt).Substring($idx) }
        }
    }
    Set-Content -LiteralPath $K.Roh -Value (($kopf -join "`n") + $szenarienTeil) -Encoding utf8
}

function Ist-Ende($fehler) { return ([string]$fehler.Exception.Message -eq $script:EndeMarke) }

function Raeume-Anfrage-Ab {
    if (-not $script:K.DiagnoseOrdner) { return }
    $anfrage = Join-Path $script:K.DiagnoseOrdner 'anfrage.json'
    if (& $script:U.Existiert $anfrage) {
        & $script:U.Loesche $anfrage
        Log 'Briefkasten: anfrage.json abgeraeumt'
    }
}

function Pruefe-Projekt {
    $K = $script:K
    if (-not $K.ProjektHashes) { return $null }
    $repo = & $script:U.Hash $K.Projekt
    $kopie = & $script:U.Hash (Join-Path $K.ProjektOrdner $K.ProjektName)
    Log ("Diagnoseprojekt am Ende: SHA-256 Repo {0}, Arbeitskopie {1}" -f $repo, $kopie)
    $gruende = @()
    if ($repo -ne $K.ProjektHashes.Repo -or $kopie -ne $K.ProjektHashes.Kopie) {
        $gruende += "Diagnoseprojekt veraendert (Repo $($K.ProjektHashes.Repo) -> $repo, Arbeitskopie $($K.ProjektHashes.Kopie) -> $kopie)"
    }
    # P-18: M-12 gilt fuer alle drei Projekte - jedes kopierte Referenzprojekt wird wie das Diagnoseprojekt verglichen.
    foreach ($name in @($K.ReferenzHashes.Keys)) {
        $h = $K.ReferenzHashes[$name]
        $r = & $script:U.Hash $h.Quelle
        $c = & $script:U.Hash $h.Ziel
        Log ("Referenzprojekt {0} am Ende: SHA-256 Repo {1}, Arbeitskopie {2}" -f $name, $r, $c)
        if ($r -ne $h.Repo -or $c -ne $h.Kopie) { $gruende += "Referenzprojekt $name veraendert (Repo $($h.Repo) -> $r, Arbeitskopie $($h.Kopie) -> $c)" }
    }
    if ($gruende.Count -gt 0) { return ($gruende -join '; ') }
    return $null
}

function Ende([int]$code, [string]$urteil, [string]$zusatz = '') {
    $K = $script:K
    if ($K.EndeLaeuft) { throw $script:EndeMarke }
    $K.EndeLaeuft = $true
    try { Raeume-Anfrage-Ab } catch { Log "Anfrage abraeumen fehlgeschlagen: $($_.Exception.Message)" }
    $veraendert = $null
    try { $veraendert = Pruefe-Projekt } catch { Log "Projekt-Hash am Ende nicht bestimmbar: $($_.Exception.Message)" }
    if ($veraendert) { $code = 4; $urteil = 'VERFEHLT'; $zusatz = ("$zusatz $veraendert").Trim() }
    Schreibe-Roh $urteil $zusatz
    $kopfzeile = "LAUFZEIT {0} {1} {2} {3} roh={4}" -f $K.Ticket, $K.Head, $urteil, $zusatz, $K.Roh
    if (-not $K.Leise) { Write-Host $kopfzeile }
    $K.Kopfzeilen.Add($kopfzeile)
    $K.Ergebnis = [pscustomobject]@{ Code = $code; Urteil = $urteil; Zusatz = $zusatz; Kopfzeile = $kopfzeile }
    throw $script:EndeMarke
}

# ---------------------------------------------------------------- Echte Umgebung
function Lade-Fensterliste {
    if ('NakamaFensterliste' -as [type]) { return }
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public static class NakamaFensterliste
{
    delegate bool EnumProc(IntPtr hwnd, IntPtr lparam);
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr lparam);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint pid);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetClassNameW(IntPtr hwnd, StringBuilder text, int size);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetWindowTextW(IntPtr hwnd, StringBuilder text, int size);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);

    // Klasse, Titel und Sichtbarkeit aller Hauptfenster eines Prozesses.
    public static List<string[]> Liste(uint pid)
    {
        var liste = new List<string[]>();
        EnumWindows((hwnd, lparam) =>
        {
            uint besitzer;
            GetWindowThreadProcessId(hwnd, out besitzer);
            if (besitzer == pid)
            {
                var klasse = new StringBuilder(256);
                GetClassNameW(hwnd, klasse, 256);
                var titel = new StringBuilder(512);
                GetWindowTextW(hwnd, titel, 512);
                liste.Add(new[] { klasse.ToString(), titel.ToString(), IsWindowVisible(hwnd) ? "1" : "0" });
            }
            return true;
        }, IntPtr.Zero);
        return liste;
    }
}
'@
}

function Neue-Umgebung {
    $u = @{}
    $u.Jetzt = { Get-Date }
    $u.Schlafe = { param([double]$s) if ($s -gt 0) { Start-Sleep -Milliseconds ([int][math]::Ceiling($s * 1000)) } }
    $u.Prozesse = {
        @(Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue | ForEach-Object {
            [pscustomobject]@{ Id = $_.Id; Name = $_.ProcessName; MainWindowTitle = [string]$_.MainWindowTitle; MainWindowHandle = [int64]$_.MainWindowHandle }
        })
    }
    $u.Fenster = {
        param([int]$id)
        Lade-Fensterliste
        @([NakamaFensterliste]::Liste([uint32]$id) | ForEach-Object { [pscustomobject]@{ Klasse = $_[0]; Titel = $_[1]; Sichtbar = ($_[2] -eq '1') } })
    }
    $u.BeendeProzess = { param([int]$id) Stop-Process -Id $id -Force -ErrorAction SilentlyContinue }
    $u.Git = {
        param([string[]]$argumente)
        $zeilen = @(& git -C $script:K.Repo @argumente 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
    $u.Py = {
        param([string[]]$argumente)
        $zeilen = @(& py -3.13 @argumente 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
    $u.AufgabeStarte = {
        param([string]$name)
        $t = Get-ScheduledTask -TaskPath '\Nakama\' -TaskName $name -ErrorAction SilentlyContinue
        if (-not $t) { throw "Aufgabe \Nakama\$name fehlt - einmal erhoeht tools/dirigent/install-aufgaben-registrieren.ps1 ausfuehren (NAK-285)" }
        Start-ScheduledTask -TaskPath '\Nakama\' -TaskName $name
    }
    $u.AufgabeZustand = { param([string]$name) [string](Get-ScheduledTask -TaskPath '\Nakama\' -TaskName $name).State }
    $u.AufgabeErgebnis = { param([string]$name) [int](Get-ScheduledTaskInfo -TaskPath '\Nakama\' -TaskName $name).LastTaskResult }
    $u.AufgabeLog = {
        param([string]$name)
        $log = Join-Path $env:LOCALAPPDATA "evenacadia\nakama-installer\$name.log"
        if (Test-Path -LiteralPath $log) { @(Get-Content -LiteralPath $log) } else { @() }
    }
    $u.LoopMidiLaeuft = { [bool](Get-Process loopMIDI -ErrorAction SilentlyContinue) }
    $u.StarteLoopMidi = { param([string]$pfad) Start-Process -FilePath $pfad | Out-Null }
    $u.Existiert = { param([string]$pfad) Test-Path -LiteralPath $pfad }
    $u.Kopiere = { param([string]$von, [string]$nach) Copy-Item -LiteralPath $von -Destination $nach -Force }
    $u.LegeOrdnerAn = { param([string]$pfad) New-Item -ItemType Directory -Force -Path $pfad | Out-Null }
    $u.Hash = { param([string]$pfad) if (Test-Path -LiteralPath $pfad) { (Get-FileHash -Algorithm SHA256 -LiteralPath $pfad).Hash } else { $null } }
    $u.Namen = { param([string]$ordner) if (Test-Path -LiteralPath $ordner) { @(Get-ChildItem -LiteralPath $ordner -File -Force | ForEach-Object { $_.Name }) } else { @() } }
    $u.Loesche = { param([string]$pfad) [IO.File]::Delete($pfad) }
    $u.LiesText = { param([string]$pfad) if (Test-Path -LiteralPath $pfad) { [IO.File]::ReadAllText($pfad) } else { $null } }
    $u.SchreibeText = { param([string]$pfad, [string]$text) [IO.File]::WriteAllText($pfad, $text, (New-Object System.Text.UTF8Encoding($false))) }
    $u.FlPfad = {
        try { $p = (Get-ItemProperty 'HKCU:\Software\Image-Line\Shared\Paths' -ErrorAction Stop).'FL Studio'; if ($p -and (Test-Path -LiteralPath $p)) { return $p } } catch { }
        'C:\Program Files\Image-Line\FL Studio 2026\FL64.exe'
    }
    $script:GestarteteProzesse = @{}
    $u.StarteProzess = {
        param([string]$exe, [string]$argumente)
        # Eine vorab gequotete Zeichenkette: Start-Process quotiert Argumente nicht selbst (CLAUDE.md, Landminen).
        $p = Start-Process -FilePath $exe -ArgumentList $argumente -PassThru
        $null = $p.Handle
        $script:GestarteteProzesse[$p.Id] = $p
        $p.Id
    }
    $u.ProzessLaeuft = { param([int]$id) $p = $script:GestarteteProzesse[$id]; if ($p) { -not $p.HasExited } else { [bool](Get-Process -Id $id -ErrorAction SilentlyContinue) } }
    $u.ProzessExit = { param([int]$id) $p = $script:GestarteteProzesse[$id]; if ($p -and $p.HasExited) { $p.ExitCode } else { $null } }
    $u.BootMarke = {
        $pfad = Join-Path (Dokumente-Ordner) 'Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\mcp_boot.json'
        if (-not (Test-Path -LiteralPath $pfad)) { return @{ Pfad = $pfad; Existiert = $false } }
        $inhalt = (Get-Content -LiteralPath $pfad -Raw) -replace '\s+', ' '
        $version = $null
        try { $version = ((Get-Content -LiteralPath $pfad -Raw) | ConvertFrom-Json).script_version } catch { }
        @{ Pfad = $pfad; Existiert = $true; Zeit = (Get-Item -LiteralPath $pfad).LastWriteTime; Inhalt = $inhalt; Version = $version }
    }
    $u.ControllerInstalliert = { Join-Path (Dokumente-Ordner) 'Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py' }
    $u.SetupLocal = {
        $zeilen = @(& pwsh -NoProfile -File (Join-Path $script:K.McpRepo 'setup-local.ps1') 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
    $u.Ping = {
        $ausgabe = & uv run --directory $script:K.McpRepo --python 3.12 python $script:K.SzenarioPy --ping 2>&1
        $zeile = @($ausgabe | Where-Object { $_ -like '{*' }) | Select-Object -Last 1
        if (-not $zeile) { return @{ Ok = $false; Fehler = ($ausgabe -join ' ').Trim() } }
        try { $j = $zeile | ConvertFrom-Json } catch { return @{ Ok = $false; Fehler = $zeile } }
        if (-not $j.success) { return @{ Ok = $false; Fehler = $j.error } }
        @{ Ok = $true; Titel = $j.program_title; Version = $j.fl_version; Port = $j.port_name }
    }
    $u.FahreSzenario = {
        param([string]$pfad)
        $argumente = @($script:K.SzenarioPy, $pfad, '--roh', $script:K.Roh, '--render-ordner', $script:K.RenderOrdner,
            '--diagnose-ordner', $script:K.DiagnoseOrdner, '--projekt-ordner', $script:K.ProjektOrdner, '--head', $script:K.Head)
        if ($script:K.DiagnosePid) { $argumente += @('--diagnose-pid', [string]$script:K.DiagnosePid) }
        $zeilen = @(& uv run --directory $script:K.McpRepo --python 3.12 python @argumente 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
    return $u
}

function Dokumente-Ordner {
    $basis = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders' -ErrorAction SilentlyContinue).Personal
    if ($basis) { return [Environment]::ExpandEnvironmentVariables($basis) }
    return (Join-Path $env:USERPROFILE 'Documents')
}

# ---------------------------------------------------------------- Hilfen
function FL-Instanzen {
    # NAK-297: der Titel eines FL-Prozesses kommt aus seinem Hauptfenster der Klasse
    # TFruityLoopsMainForm; MainWindowTitle liefert unter FL 2026 manchmal das leere
    # TApplication-Fenster. Ein leerer Titel ist kein Fremdprojekt.
    foreach ($p in @(& $script:U.Prozesse)) {
        $haupt = @(& $script:U.Fenster $p.Id | Where-Object { $_.Klasse -eq 'TFruityLoopsMainForm' -and $_.Sichtbar })
        $titel = ''
        if ($haupt.Count -gt 0 -and $haupt[0].Titel) { $titel = [string]$haupt[0].Titel }
        elseif ($p.MainWindowTitle) { $titel = [string]$p.MainWindowTitle }
        [pscustomobject]@{ Id = $p.Id; Name = $p.Name; Titel = $titel; Fenster = ($haupt.Count -gt 0 -or [int64]$p.MainWindowHandle -ne 0) }
    }
}
function Ist-Diagnose($inst) { $inst.Fenster -and $inst.Titel -like "*$($script:K.ProjektName)*" }
function Ist-Fremd($inst)    { $inst.Fenster -and $inst.Titel -and -not ($inst.Titel -like "*$($script:K.ProjektName)*") }

function Beende-Diagnose-FL {
    foreach ($i in @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })) {
        Log "Diagnose-FL beenden: PID $($i.Id) '$($i.Titel)' (Projekt wird nie gespeichert)"
        & $script:U.BeendeProzess $i.Id
    }
    $t0 = & $script:U.Jetzt
    while (@(FL-Instanzen | Where-Object { Ist-Diagnose $_ }).Count -gt 0 -and ((& $script:U.Jetzt) - $t0).TotalSeconds -lt 30) { & $script:U.Schlafe 1 }
}
function Beende-Restprozesse {
    foreach ($i in @(FL-Instanzen | Where-Object { -not $_.Fenster })) {
        & $script:U.BeendeProzess $i.Id
        & $script:U.Schlafe 0.5
        # Get-Process -Id schweigt bei Zugriffsverweigerung; die Liste luegt nicht.
        $weg = -not (@(FL-Instanzen) | Where-Object { $_.Id -eq $i.Id })
        Log ("FL-Restprozess PID {0}: {1}" -f $i.Id, $(if ($weg) { 'beendet' } else { 'nicht beendbar (Zugriff), ignoriert' }))
    }
}

function Boot-Marke-Text {
    $m = & $script:U.BootMarke
    if (-not $m.Existiert) { return "keine Boot-Marke ($($m.Pfad))" }
    return ("Boot-Marke {0} ({1})" -f $m.Zeit.ToString('HH:mm:ss'), $m.Inhalt)
}

function Lohnt-Es {
    $K = $script:K
    if ($K.Erzwingen) { return @{ Ja = $true; Grund = 'erzwungen (-Erzwingen)' } }
    if (-not $K.Basis) { return @{ Ja = $true; Grund = 'ohne Basis-SHA: immer' } }
    $r = & $script:U.Git @('diff', '--name-only', "$($K.Basis)..HEAD")
    $dateien = @($r.Zeilen | Where-Object { $_ })
    if ($r.Exit -ne 0) { return @{ Ja = $true; Grund = "Diff nicht bestimmbar ($($dateien -join ' ')): vorsichtshalber ja" } }
    $treffer = @($dateien | Where-Object { $d = $_; $script:Produktpfade | Where-Object { $d.StartsWith($_) } })
    if ($treffer.Count -gt 0) { return @{ Ja = $true; Grund = "Produktpfade im Diff: $($treffer.Count) Datei(en), z. B. $($treffer[0])" } }
    return @{ Ja = $false; Grund = "Diff $($K.Basis)..HEAD ($($dateien.Count) Datei(en)) beruehrt keinen Produktpfad" }
}

function Aufgabe([string]$name) {
    & $script:U.AufgabeStarte $name
    $t0 = & $script:U.Jetzt
    do {
        & $script:U.Schlafe 1
        $zustand = & $script:U.AufgabeZustand $name
    } while ($zustand -eq 'Running' -and ((& $script:U.Jetzt) - $t0).TotalSeconds -lt $script:K.InstallFristSekunden)
    $exit = & $script:U.AufgabeErgebnis $name
    $zeilen = @(& $script:U.AufgabeLog $name)
    $tail = if ($zeilen.Count -gt 0) { ($zeilen | Select-Object -Last 4) -join ' | ' } else { '(kein Log)' }
    return @{ Exit = $exit; Zustand = $zustand; Log = $tail; Zeilen = $zeilen }
}

function Installiere {
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { return @{ Ok = $false; Ueberspringen = $true; Grund = "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" } }
    Beende-Diagnose-FL
    Beende-Restprozesse
    Log 'Manifest-Hashes nachziehen (--hashen)'
    $h = & $script:U.Py @((Join-Path $script:K.Repo 'tools\eq-copilot\pruefe_installer_manifest.py'), '--hashen')
    foreach ($z in @($h.Zeilen)) { Log "  hashen: $z" }
    if ($h.Exit -ne 0) { return @{ Ok = $false; Ueberspringen = $false; Grund = "--hashen Exit $($h.Exit)" } }
    Log 'Aufgabe \Nakama\installieren starten'
    $r = Aufgabe 'installieren'
    Log ("  installieren: Exit {0}, Zustand {1}, Log: {2}" -f $r.Exit, $r.Zustand, $r.Log)
    if ($r.Exit -ne 0 -or $r.Zustand -eq 'Running') { return @{ Ok = $false; Ueberspringen = $false; Grund = "Installation Exit $($r.Exit) ($($r.Zustand))" } }
    $p = Aufgabe 'pruefen'
    Log ("  pruefen: Exit {0}, Zustand {1}, Log: {2}" -f $p.Exit, $p.Zustand, $p.Log)
    # M-06: nur "aktuell" fuer jedes Artefakt laesst den Lauf weiter - kein FL-Start gegen
    # einen ungeprueften Bau (Install-Nakama.ps1 -Pruefen: aktuell, ABWEICHEND, nicht installiert).
    $stand = @($p.Zeilen | Where-Object { $_ -match '^\s{2}(aktuell|ABWEICHEND|nicht installiert)\s+\S' })
    $nichtAktuell = @($stand | Where-Object { $_ -notmatch '^\s{2}aktuell\s' })
    if ($p.Exit -ne 0 -or $p.Zustand -eq 'Running' -or $stand.Count -eq 0 -or $nichtAktuell.Count -gt 0) {
        $befund = if ($nichtAktuell.Count -gt 0) { ($nichtAktuell | ForEach-Object { $_.Trim() }) -join ' | ' } elseif ($stand.Count -eq 0) { 'kein Artefaktstand im Log' } else { $p.Log }
        return @{ Ok = $false; Ueberspringen = $false; Grund = "installiert, aber nicht aktuell: pruefen Exit $($p.Exit) ($($p.Zustand)); $befund" }
    }
    return @{ Ok = $true; Ueberspringen = $false; Grund = "installiert; pruefen Exit $($p.Exit), $($stand.Count) Artefakt(e) aktuell" }
}

function Pruefe-Controller {
    # M-09: das Controller-Skript des MCP-Repos gegen die installierte Kopie (SHA-256). Bei
    # Abweichung setup-local.ps1, ein laufendes Diagnose-FL beenden (nie ein fremdes, nie
    # speichern) und nach dem Neustart eine Boot-Marke mit der script_version des Repo-Stands
    # verlangen (Warte-Boot).
    $K = $script:K
    $repoSkript = Join-Path $K.McpRepo 'fl_controller\device_FLStudioMCP.py'
    if (-not (& $script:U.Existiert $repoSkript)) { throw "Controller-Skript fehlt im MCP-Repo: $repoSkript" }
    $installiert = & $script:U.ControllerInstalliert
    $hRepo = & $script:U.Hash $repoSkript
    $hInstalliert = & $script:U.Hash $installiert
    $treffer = [regex]::Match([string](& $script:U.LiesText $repoSkript), 'SCRIPT_VERSION\s*=\s*"([^"]+)"')
    $K.ScriptVersion = if ($treffer.Success) { $treffer.Groups[1].Value } else { $null }
    Log ("Controller-Skript: SHA-256 Repo {0}, installiert {1} ({2}), script_version {3}" -f $hRepo, $hInstalliert, $installiert, $K.ScriptVersion)
    if ($hRepo -eq $hInstalliert) { return }
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen vor der Neuinstallation des Controllers: '$($fremd[0].Titel)'" }
    Beende-Diagnose-FL
    $r = & $script:U.SetupLocal
    foreach ($z in @($r.Zeilen)) { Log "  setup-local: $z" }
    if ($r.Exit -ne 0) { throw "setup-local.ps1 Exit $($r.Exit)" }
    $neu = & $script:U.Hash $installiert
    if ($neu -ne $hRepo) { throw "Controller nach setup-local.ps1 weiter veraltet: installiert $neu, Repo $hRepo" }
    $K.ControllerNeu = $true
    $K.Neustart = $true
    Log "Controller-Skript neu installiert (script_version $($K.ScriptVersion)); FL-Neustart folgt"
}

function Stelle-LoopMidi-Sicher {
    if (& $script:U.LoopMidiLaeuft) { Log 'loopMIDI laeuft'; return }
    if (-not (& $script:U.Existiert $script:K.LoopMidi)) { throw "loopMIDI fehlt unter $($script:K.LoopMidi)" }
    & $script:U.StarteLoopMidi $script:K.LoopMidi
    & $script:U.Schlafe 3
    Log 'loopMIDI gestartet (Port aus der Registry wird beim Start angelegt)'
}

function Ist-Projektname([string]$name) {
    # P-18: ein Dateiname ohne Pfadanteil mit Endung .flp - dieselbe Regel fuer Kopie (Bereite-Projekt), Render
    # (Rendere-Referenzprojekte) und Referenzschritt (szenario.py, ist_projektname).
    return ([bool]$name -and [IO.Path]::GetFileName($name) -eq $name -and $name -like '*.flp')
}

function Referenzprojekt-Namen([string[]]$liste) {
    # P-18 (§38.2): die Namen der Referenzprojekte aus Karte U43 stehen in der Szenariodatei (Schritte lokal.nulltest,
    # params.projekt) - kein anderer Name, kein Suchen nach Mustern.
    $namen = New-Object System.Collections.Generic.List[string]
    foreach ($s in @($liste)) {
        try { $szenario = (& $script:U.LiesText $s) | ConvertFrom-Json } catch { continue }
        foreach ($schritt in @($szenario.schritte)) {
            if (-not $schritt -or $schritt.aktion -ne 'lokal.nulltest' -or -not $schritt.params) { continue }
            $name = [string]$schritt.params.projekt
            if ($name -and -not $namen.Contains($name)) { $namen.Add($name) }
        }
    }
    return @($namen)
}

function Bereite-Projekt([string[]]$liste = @()) {
    $K = $script:K
    if (-not (& $script:U.Existiert $K.Projekt)) { throw "Diagnoseprojekt fehlt: $($K.Projekt)" }
    & $script:U.LegeOrdnerAn $K.ProjektOrdner
    $kopie = Join-Path $K.ProjektOrdner $K.ProjektName
    & $script:U.Kopiere $K.Projekt $kopie
    $samples = Join-Path $K.ProjektOrdner 'Samples'
    & $script:U.LegeOrdnerAn $samples
    $ziel = Join-Path $samples 'Testtrack.wav'
    if (-not (& $script:U.Existiert $ziel)) {
        # Testtrack.wav ist ein Maschinenartefakt (nicht im Git); Kandidaten
        # in dieser Reihenfolge, -Sample ueberschreibt.
        $kandidaten = @(@($K.Sample) + @(
            (Join-Path $K.Repo 'eq-copilot\kalibration\Testtrack.wav'),
            'C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav',
            (Join-Path $env:USERPROFILE 'OneDrive\Desktop\Skripte\Project_1\Samples\Testtrack.wav')
        ) | Where-Object { $_ -and (& $script:U.Existiert $_) })
        if ($kandidaten.Count -eq 0) { throw "Testtrack.wav fehlt (kein Kandidat vorhanden); -Sample <pfad> angeben oder nach $samples legen" }
        & $script:U.Kopiere $kandidaten[0] $ziel
        Log "Testtrack.wav aus $($kandidaten[0]) nach $samples kopiert"
    }
    # M-12: SHA-256 beider Projektdateien nach dem Kopieren; jeder Ausgang vergleicht sie (Ende).
    $K.ProjektHashes = @{ Repo = (& $script:U.Hash $K.Projekt); Kopie = (& $script:U.Hash $kopie) }
    Log ("Diagnoseprojekt: SHA-256 Repo {0}, Arbeitskopie {1}" -f $K.ProjektHashes.Repo, $K.ProjektHashes.Kopie)
    # P-18, M-12 fuer alle drei Projekte: die Referenzprojekte aus Karte U43 unter genau den Namen der Szenariodatei, im
    # Ordner des Diagnoseprojekts; kopiert wie das Diagnoseprojekt, SHA-256 von Quelle und Arbeitskopie, jeder Ausgang
    # vergleicht sie (Pruefe-Projekt). Ein fehlendes Projekt wird nicht kopiert; lokal.nulltest meldet es (M-64).
    $K.ReferenzHashes = [ordered]@{}
    $ordner = [IO.Path]::GetDirectoryName($K.Projekt)
    foreach ($name in @(Referenzprojekt-Namen $liste)) {
        if (-not (Ist-Projektname $name)) {
            Log "Referenzprojekt '$name': kein Projektdateiname im Ordner des Diagnoseprojekts, nicht kopiert"
            continue
        }
        $refQuelle = Join-Path $ordner $name
        if (-not (& $script:U.Existiert $refQuelle)) {
            Log "Referenzprojekt ${name}: fehlt neben dem Diagnoseprojekt ($refQuelle; Karte U43), nicht kopiert"
            continue
        }
        $refKopie = Join-Path $K.ProjektOrdner $name
        & $script:U.Kopiere $refQuelle $refKopie
        $K.ReferenzHashes[$name] = @{ Quelle = $refQuelle; Ziel = $refKopie; Repo = (& $script:U.Hash $refQuelle); Kopie = (& $script:U.Hash $refKopie) }
        Log ("Referenzprojekt {0}: SHA-256 Repo {1}, Arbeitskopie {2}" -f $name, $K.ReferenzHashes[$name].Repo, $K.ReferenzHashes[$name].Kopie)
    }
    return $kopie
}

function Bereite-Briefkasten {
    # M-55: der Runner besitzt den Briefkasten. Er legt diagnose\ und antwort\ an und
    # entfernt vor der ersten Anfrage zuerst eine liegende Anfrage, dann Antworten
    # frueherer Laeufe - nur Antwort- und Temp-Namen der Instanzen (§10.2 P-1).
    $K = $script:K
    & $script:U.LegeOrdnerAn $K.DiagnoseOrdner
    & $script:U.LegeOrdnerAn $K.AntwortOrdner
    $anfrage = Join-Path $K.DiagnoseOrdner 'anfrage.json'
    if (& $script:U.Existiert $anfrage) {
        & $script:U.Loesche $anfrage
        Log 'Briefkasten: liegende anfrage.json entfernt'
    }
    $n = 0
    foreach ($name in @(& $script:U.Namen $K.AntwortOrdner)) {
        if ($name -match $script:AntwortMuster -or $name -match $script:TempMuster) {
            & $script:U.Loesche (Join-Path $K.AntwortOrdner $name)
            $n++
        }
    }
    Log "Briefkasten: $($K.DiagnoseOrdner) bereit, $n Antwortdatei(en) frueherer Laeufe entfernt"
}

function Rendere([string]$flp, [string]$ordner = $script:K.RenderOrdner, [string]$bezeichnung = 'Render') {
    # M-60, F-18: Render ueber die Kommandozeile des FL-Handbuchs (/R, /E, /O), nur ohne FL mit Fenster; Frist, danach
    # genau diese PID beenden. P-21 (a): dieselbe Folge fuer das Diagnoseprojekt (Ordner render\, Bezeichnung Render)
    # und jedes Referenzprojekt (Ordner render\referenz\<Name>\, Bezeichnung "Referenzrender <Name>").
    $K = $script:K
    & $script:U.LegeOrdnerAn $ordner
    $wav = Join-Path $ordner ([IO.Path]::ChangeExtension([IO.Path]::GetFileName($flp), '.wav'))
    foreach ($alt in @($wav, (Join-Path $ordner 'ergebnis.json'), (Join-Path $ordner 'render.json'))) {
        if (& $script:U.Existiert $alt) { & $script:U.Loesche $alt }
    }
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen vor dem Render: '$($fremd[0].Titel)'" }
    Beende-Diagnose-FL
    Beende-Restprozesse
    $status = [ordered]@{ format = 'nakama.laufzeit.render.v1'; head = $K.Head; projekt = $flp; sha256_projekt = (& $script:U.Hash $flp); datei = $null; grund = $null }
    $rest = @(FL-Instanzen)
    if ($rest.Count -gt 0) {
        $status.grund = 'FL laeuft vor dem Render: ' + (($rest | ForEach-Object { "PID $($_.Id) '$($_.Titel)'" }) -join ', ')
    } else {
        $exe = & $script:U.FlPfad
        $argumente = '/R /Ewav /O"' + $ordner + '" "' + $flp + '"'
        $t0 = & $script:U.Jetzt
        $renderPid = & $script:U.StarteProzess $exe $argumente
        Log "$bezeichnung gestartet: PID $renderPid ($exe $argumente)"
        while ((& $script:U.ProzessLaeuft $renderPid) -and ((& $script:U.Jetzt) - $t0).TotalSeconds -lt $K.RenderFristSekunden) { & $script:U.Schlafe 1 }
        $titel = ''
        $haengt = [bool](& $script:U.ProzessLaeuft $renderPid)
        if ($haengt) {
            $titel = (@(& $script:U.Fenster $renderPid | Where-Object { $_.Sichtbar -and $_.Titel } | ForEach-Object { "$($_.Klasse): $($_.Titel)" }) -join ' | ')
            Log "$bezeichnung haengt nach $($K.RenderFristSekunden) s: genau PID $renderPid beenden (Fenster: '$titel'; eigene Arbeitskopie, nie gespeichert)"
            & $script:U.BeendeProzess $renderPid
        }
        $status.pid = $renderPid
        $status.aufruf = "$exe $argumente"
        $status.dauer_s = [math]::Round(((& $script:U.Jetzt) - $t0).TotalSeconds, 1)
        $status.exit = & $script:U.ProzessExit $renderPid
        $status.fenstertitel = $titel
        if ($haengt) { $status.grund = "Prozess haengt (Frist $($K.RenderFristSekunden) s), Fenster '$titel'" }
        elseif (& $script:U.Existiert $wav) { $status.datei = $wav; $status.sha256 = & $script:U.Hash $wav }
        else { $status.grund = "keine Datei: $wav fehlt nach Exit $($status.exit)" }
    }
    $status.zeit = Zeit
    & $script:U.SchreibeText (Join-Path $ordner 'render.json') ($status | ConvertTo-Json -Depth 4)
    if ($bezeichnung -ne 'Render') {
        Log ("{0}: Exit {1}, Dauer {2} s, Datei {3}, SHA-256 {4}, SHA-256 Projekt {5}, Grund {6}" -f $bezeichnung, $status.exit, $status.dauer_s, $status.datei, $status.sha256, $status.sha256_projekt, $status.grund)
        return
    }
    Log ("Render: Exit {0}, Dauer {1} s, Datei {2}, SHA-256 {3}, Grund {4}" -f $status.exit, $status.dauer_s, $status.datei, $status.sha256, $status.grund)
}

function Nenne-Renderfolge([string[]]$liste) {
    # P-21 (a): die Reihenfolge der Renders vor dem FL-Start steht vorab im Protokoll - zuerst das Diagnoseprojekt im
    # Auslieferungszustand (nur mit nulltest-host.json), danach die Referenzprojekte in der Reihenfolge der Szenariodatei.
    $K = $script:K
    $folge = New-Object System.Collections.Generic.List[string]
    if (@($liste | Where-Object { [IO.Path]::GetFileName($_) -eq 'nulltest-host.json' }).Count -gt 0) { $folge.Add("$($K.ProjektName) (Auslieferungszustand)") }
    foreach ($projektname in @(Referenzprojekt-Namen $liste)) {
        if (-not (Ist-Projektname $projektname)) { $folge.Add("'$projektname' (kein Projektdateiname, kein Render)") }
        elseif ($K.ReferenzHashes.Contains($projektname)) { $folge.Add("$projektname (Referenzprojekt)") }
        else { $folge.Add("$projektname (Referenzprojekt fehlt, kein Render)") }
    }
    if ($folge.Count -eq 0) { return }
    Log ('Renderfolge vor dem FL-Start: ' + ((0..($folge.Count - 1) | ForEach-Object { "$($_ + 1) $($folge[$_])" }) -join ', '))
}

function Rendere-Referenzprojekte([string[]]$liste) {
    # P-21 (a) (§44.3): nach dem Render des Diagnoseprojekts und vor dem FL-Start jedes kopierte Referenzprojekt aus Karte
    # U43 in der Reihenfolge der Szenariodatei wie das Diagnoseprojekt (F-18) aus der Arbeitskopie rendern, je Projekt in
    # einen eigenen Ordner render\referenz\<Name>\ mit eigenem render.json und eigener Renderdatei - Ort und Name des
    # Auslieferungsrenders, seines render.json und seiner ergebnis.json bleiben (F-26). Ein fehlendes Referenzprojekt
    # bekommt dort einen Renderstatus mit Grund und keinen Render; lokal.nulltest liest ihn (M-64).
    $K = $script:K
    foreach ($projektname in @(Referenzprojekt-Namen $liste)) {
        if (-not (Ist-Projektname $projektname)) { Log "Referenzrender '$projektname': kein Projektdateiname, kein Renderstatus"; continue }
        $ordner = Join-Path $K.RenderOrdner "referenz\$projektname"
        $kopie = Join-Path $K.ProjektOrdner $projektname
        if ($K.ReferenzHashes.Contains($projektname)) { Rendere $kopie $ordner "Referenzrender $projektname"; continue }
        & $script:U.LegeOrdnerAn $ordner
        foreach ($frueher in @((Join-Path $ordner ([IO.Path]::ChangeExtension($projektname, '.wav'))), (Join-Path $ordner 'ergebnis.json'), (Join-Path $ordner 'render.json'))) {
            if (& $script:U.Existiert $frueher) { & $script:U.Loesche $frueher }
        }
        $statusDatei = Join-Path $ordner 'render.json'
        $fehlend = [ordered]@{ format = 'nakama.laufzeit.render.v1'; head = $K.Head; projekt = $kopie; sha256_projekt = $null; datei = $null; grund = $script:ReferenzFehlt; zeit = (Zeit) }
        & $script:U.SchreibeText $statusDatei ($fehlend | ConvertTo-Json -Depth 4)
        Log "Referenzrender ${projektname}: kein Render - $($script:ReferenzFehlt); Renderstatus $statusDatei"
    }
}

function Starte-FL([string]$flp) {
    $exe = & $script:U.FlPfad
    $script:K.FlStart = & $script:U.Jetzt
    $id = & $script:U.StarteProzess $exe ('"' + $flp + '"')
    $script:K.DiagnosePid = $id
    Log "FL gestartet: PID $id ($exe) mit $flp"
}

function Warte-Boot {
    # Gemessen 12.09.2026: ein Trigger, der FL waehrend der Initialisierung
    # trifft, laesst die Instanz danach dauerhaft stumm. Deshalb erst auf die
    # Boot-Marke des Controller-Skripts (OnInit) warten, dann eine Schonfrist
    # fuer das Laden des Projekts - und vorher KEINEN einzigen Trigger senden.
    $K = $script:K
    $t0 = & $script:U.Jetzt
    while (((& $script:U.Jetzt) - $t0).TotalSeconds -lt 60) {
        $m = & $script:U.BootMarke
        if ($m.Existiert -and $m.Zeit -ge $K.FlStart) {
            Log ("Boot-Marke nach {0} s: {1}" -f [int]((& $script:U.Jetzt) - $t0).TotalSeconds, $m.Inhalt)
            if ($K.ControllerNeu -and $m.Version -ne $K.ScriptVersion) { throw "Boot-Marke nach der Neuinstallation traegt script_version '$($m.Version)' statt '$($K.ScriptVersion)'" }
            & $script:U.Schlafe 8
            return
        }
        & $script:U.Schlafe 1
    }
    if ($K.ControllerNeu) { throw "keine Boot-Marke binnen 60 s nach der Neuinstallation des Controllers (script_version $($K.ScriptVersion))" }
    Log 'keine Boot-Marke binnen 60 s (altes Controller-Skript?); Schonfrist 30 s'
    & $script:U.Schlafe 30
}

function Warte-Ping {
    # ui.getProgTitle() liefert nur "FL Studio 2026" (gemessen 12.09.2026);
    # welches Projekt offen ist, sagt das Hauptfenster der Instanz (FL-Instanzen).
    $K = $script:K
    $t0 = & $script:U.Jetzt
    $letzter = ''
    while (((& $script:U.Jetzt) - $t0).TotalSeconds -lt $K.PingFristSekunden) {
        $r = & $script:U.Ping
        if ($r.Ok) {
            $inst = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
            if ($inst.Count -gt 0) {
                $K.DiagnosePid = $inst[0].Id
                Log ("Ping ok: FL {0} ('{1}'), Fenster '{2}', Port {3}, Diagnose-PID {4}" -f $r.Version, $r.Titel, $inst[0].Titel, $r.Port, $inst[0].Id)
                return $r
            }
            $letzter = "Ping antwortet, aber kein Fenster mit $($K.ProjektName)"
        } else { $letzter = $r.Fehler }
        & $script:U.Schlafe 5
    }
    throw "kein Ping binnen $($K.PingFristSekunden) s: $letzter; $(Boot-Marke-Text)"
}

function Frischer-Start([string]$name) {
    # M-59: das Szenario braucht eine frische Messung (K5, T-13); nie gespeichert.
    Log "Szenario ${name}: frischer_start - Diagnose-FL neu starten"
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen vor dem frischen Start: '$($fremd[0].Titel)'" }
    Beende-Diagnose-FL
    Beende-Restprozesse
    Starte-FL (Join-Path $script:K.ProjektOrdner $script:K.ProjektName)
    Warte-Boot
    $script:K.Ping = Warte-Ping
}

function Nulltest-Urteil {
    $text = & $script:U.LiesText (Join-Path $script:K.RenderOrdner 'ergebnis.json')
    if (-not $text) { return $null }
    try { return [string]($text | ConvertFrom-Json).urteil } catch { return $null }
}

function Fahre-Rueckweg {
    # M-65, F-20: der Rueckweg fasst bei laufendem FL nichts an (Install-Nakama.ps1).
    Log 'Rueckweg nach ABWEICHUNG: (1) Diagnose-FL beenden (nie speichern) und fensterlose Restprozesse'
    Beende-Diagnose-FL
    Beende-Restprozesse
    $laufend = @(FL-Instanzen)
    if ($laufend.Count -gt 0) {
        $grund = 'Rueckweg verweigert (FL laeuft): ' + (($laufend | ForEach-Object { "PID $($_.Id) '$($_.Titel)'" }) -join ', ')
        Log "  (2) $grund"
        return $grund
    }
    Log '  (2) kein Prozess FL64, FL oder FL Studio laeuft'
    try { $r = Aufgabe 'rueckweg' } catch {
        $meldung = "$($_.Exception.Message) (Zeile $($_.InvocationInfo.ScriptLineNumber): $($_.InvocationInfo.Line.Trim()))"
        Log "  (3) rueckweg nicht gestartet: $meldung"
        return "Rueckweg nicht gestartet: $meldung"
    }
    Log ("  (3) rueckweg: Exit {0}, Zustand {1}, Log: {2}" -f $r.Exit, $r.Zustand, $r.Log)
    $p = Aufgabe 'pruefen'
    Log ("  (4) pruefen: Exit {0}, Log: {1}" -f $p.Exit, $p.Log)
    if ((@($r.Zeilen) -join "`n") -match 'Rueckweg verweigert \(NAK-41\)') { return "Rueckweg verweigert (NAK-41); pruefen Exit $($p.Exit)" }
    return "Rueckweg Exit $($r.Exit); pruefen Exit $($p.Exit)"
}

function Szenarienliste {
    if ($script:K.Szenarien) { return @($script:K.Szenarien) }
    $ordner = Join-Path $script:K.Repo 'docs\gesundheit\szenarien'
    return @(& $script:U.Namen $ordner | Where-Object { $_ -like '*.json' } | Sort-Object | ForEach-Object { Join-Path $ordner $_ })
}

function Fahre-Szenarien([string[]]$liste) {
    $K = $script:K
    Add-Content -LiteralPath $K.Roh -Value "`n## Szenarien`n" -Encoding utf8
    $ergebnisse = @()
    $entfallen = @()
    $rueckweg = $null
    $abbruch = $null
    foreach ($s in $liste) {
        $name = [IO.Path]::GetFileName($s)
        if ($rueckweg) {
            $entfallen += $name
            Log "Szenario ${name}: entfallen: Rueckweg nach ABWEICHUNG"
            Add-Content -LiteralPath $K.Roh -Value "`n## Szenario ``$name`` - entfallen: Rueckweg nach ABWEICHUNG`n" -Encoding utf8
            continue
        }
        $kopf = $null
        try { $kopf = (& $script:U.LiesText $s) | ConvertFrom-Json } catch { }
        if ($kopf -and $kopf.frischer_start -eq $true) {
            try { Frischer-Start $name } catch {
                if (Ist-Ende $_) { throw }
                $abbruch = "frischer_start vor ${name}: $($_.Exception.Message)"
                Log "Szenario ${name}: $abbruch"
                break
            }
        }
        $r = & $script:U.FahreSzenario $s
        $code = [int]$r.Exit
        $ergebnisse += [pscustomobject]@{ Datei = $name; Exit = $code }
        Log ("Szenario {0}: Exit {1}" -f $name, $code)
        if ($code -eq 3) { break }
        if ($name -eq 'nulltest-host.json' -and (Nulltest-Urteil) -eq 'ABWEICHUNG') { $rueckweg = Fahre-Rueckweg }
    }
    return @{ Ergebnisse = $ergebnisse; Entfallen = $entfallen; Rueckweg = $rueckweg; Abbruch = $abbruch }
}

# ---------------------------------------------------------------- Ablauf
function Fahre-Lauf {
    $K = $script:K
    try {
        Log "Laufzeit-Arm $($K.Ticket) auf $($K.Head) (Basis $($K.Basis)), Repo $($K.Repo)"
        $lohnt = Lohnt-Es
        Log "Lohnt es? $($lohnt.Ja) - $($lohnt.Grund)"
        if (-not $lohnt.Ja) { Ende 0 'UEBERSPRUNGEN' $lohnt.Grund }
        $liste = @(Szenarienliste)

        $installation = 'keine (-OhneInstallation)'
        if (-not $K.OhneInstallation) {
            try { $inst = Installiere } catch { if (Ist-Ende $_) { throw }; $inst = @{ Ok = $false; Ueberspringen = $false; Grund = $_.Exception.Message } }
            if ($inst.Ueberspringen) { Ende 0 'UEBERSPRUNGEN' $inst.Grund }
            if (-not $inst.Ok) { Log "Installation fehlgeschlagen: $($inst.Grund)"; Ende 3 'VORAUSSETZUNG' "Installation: $($inst.Grund)" }
            $installation = $inst.Grund
            $K.Neustart = $true
        }
        $K.Installation = $installation

        try {
            Pruefe-Controller
            $flp = Bereite-Projekt $liste
            Bereite-Briefkasten
            Nenne-Renderfolge $liste
            if (@($liste | Where-Object { [IO.Path]::GetFileName($_) -eq 'nulltest-host.json' }).Count -gt 0) { Rendere $flp }
            Rendere-Referenzprojekte $liste
            Stelle-LoopMidi-Sicher
            Beende-Restprozesse
            $laufend = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
            $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
            if ($fremd.Count -gt 0 -and $laufend.Count -eq 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" }
            if ($K.Neustart -and $laufend.Count -gt 0) { Beende-Diagnose-FL; $laufend = @() }
            if ($laufend.Count -eq 0) { Starte-FL $flp; Warte-Boot } else { $K.DiagnosePid = $laufend[0].Id; Log "Diagnose-FL laeuft bereits: PID $($laufend[0].Id)" }
            $K.Ping = Warte-Ping
        } catch {
            if (Ist-Ende $_) { throw }
            Log "Bereitschaft fehlgeschlagen: $($_.Exception.Message)"
            Ende 3 'VORAUSSETZUNG' $_.Exception.Message
        }

        Schreibe-Roh 'LAEUFT' "installation=$installation"
        $lauf = Fahre-Szenarien $liste
        if ($K.Beenden) { Beende-Diagnose-FL }
        $ergebnisse = @($lauf.Ergebnisse)
        $verfehlt = @($ergebnisse | Where-Object { $_.Exit -ne 0 })
        $zusammen = ($ergebnisse | ForEach-Object { "{0}={1}" -f $_.Datei, $_.Exit }) -join ','
        $zusatz = "installation=$installation szenarien=$($ergebnisse.Count) verfehlt=$($verfehlt.Count) [$zusammen] fl=$($K.Ping.Version)"
        if (@($lauf.Entfallen).Count -gt 0) { $zusatz += " entfallen: $(@($lauf.Entfallen) -join ', ') (Rueckweg nach ABWEICHUNG)" }
        if ($lauf.Abbruch) { Ende 3 'VORAUSSETZUNG' "$zusatz abbruch: $($lauf.Abbruch)" }
        if ($lauf.Rueckweg) { Ende 4 'VERFEHLT' "$zusatz rueckweg: $($lauf.Rueckweg)" }
        if ($ergebnisse | Where-Object { $_.Exit -eq 3 -or $_.Exit -eq 5 }) { Ende 3 'VORAUSSETZUNG' $zusatz }
        if ($verfehlt.Count -gt 0) { Ende 4 'VERFEHLT' $zusatz }
        Ende 0 'GEMESSEN' $zusatz
    } catch {
        if (Ist-Ende $_) { return $K.Ergebnis }
        $meldung = $_.Exception.Message
        try { Log "Ausnahme: $meldung"; Ende 3 'VORAUSSETZUNG' "Ausnahme: $meldung" } catch { if (-not (Ist-Ende $_)) { throw } }
        return $K.Ergebnis
    }
}

# ---------------------------------------------------------------- Selbsttest: Attrappen (A35)
$script:T = $null
$script:SammelFehler = $null
$script:TestWurzeln = New-Object System.Collections.Generic.List[string]

function T-Protokoll([string]$eintrag) { $script:T.Protokoll.Add(("{0:HH:mm:ss.fff} {1}" -f $script:T.Uhr, $eintrag)) }

function Pruefe([bool]$bedingung, [string]$text) { if (-not $bedingung) { $script:SammelFehler.Add("[$($script:T.Name)] $text") } }
function T-Hat([string]$muster) { return (@($script:T.Protokoll | Where-Object { $_ -match $muster }).Count -gt 0) }
function T-Index([string]$muster) {
    for ($i = 0; $i -lt $script:T.Protokoll.Count; $i++) { if ($script:T.Protokoll[$i] -match $muster) { return $i } }
    return -1
}

function Neuer-Testfall([string]$name) {
    $wurzel = Join-Path ([IO.Path]::GetTempPath()) ('nak286-laufzeit-' + [guid]::NewGuid().ToString('N').Substring(0, 12))
    $script:TestWurzeln.Add($wurzel)
    $repo = Join-Path $wurzel 'repo'
    $mcp = Join-Path $wurzel 'mcp'
    foreach ($o in @((Join-Path $repo 'docs\gesundheit\szenarien'), (Join-Path $repo 'eq-copilot\fixtures\fl'),
                     (Join-Path $repo 'eq-copilot\kalibration'), (Join-Path $mcp 'fl_controller'), (Join-Path $wurzel 'settings'))) {
        New-Item -ItemType Directory -Force -Path $o | Out-Null
    }
    [IO.File]::WriteAllText((Join-Path $repo 'eq-copilot\fixtures\fl\Nakama-Diagnose.flp'), 'FLP-Attrappe')
    [IO.File]::WriteAllText((Join-Path $repo 'eq-copilot\kalibration\Testtrack.wav'), 'WAV-Attrappe')
    [IO.File]::WriteAllText((Join-Path $mcp 'fl_controller\device_FLStudioMCP.py'), "SCRIPT_VERSION = `"2026-09-15`"`n")
    $installiert = Join-Path $wurzel 'settings\device_FLStudioMCP.py'
    [IO.File]::WriteAllText($installiert, "SCRIPT_VERSION = `"2026-09-15`"`n")
    $loopMidi = Join-Path $wurzel 'loopMIDI.exe'
    [IO.File]::WriteAllText($loopMidi, 'Attrappe')
    $script:T = @{
        Name = $name; Wurzel = $wurzel; Repo = $repo; Mcp = $mcp; ControllerInstalliert = $installiert
        Uhr = [datetime]'2026-09-15T01:00:00'
        Protokoll = (New-Object System.Collections.Generic.List[string])
        Prozesse = (New-Object System.Collections.Generic.List[object]); Gestartet = @{}; NaechstePid = 5000
        Git = @{ Exit = 0; Zeilen = @('eq-copilot/plugin/src/PluginProcessor.cpp') }
        Py = @{ Exit = 0; Zeilen = @('geschrieben: eq-copilot/install/nakama-installer-v1.json') }
        Aufgaben = @{
            installieren = @{ Exit = 0; Zustand = 'Ready'; Log = @('INSTALLATION OK') }
            pruefen      = @{ Exit = 0; Zustand = 'Ready'; Log = @('Installierter Stand gegen das Manifest:', '  aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3', '  aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe') }
            rueckweg     = @{ Exit = 0; Zustand = 'Ready'; Log = @('RUECKWEG OK') }
        }
        LoopMidiLaeuft = $true
        BootNach = 5; BootVersion = '2026-09-15'; BootMarkeZeit = $null
        Ping = $null
        Szenarien = @{}
        Render = @{ Dauer = 20; Exit = 0; SchreibtDatei = $true; Haengt = $false; HaengtBei = $null; Fenster = @() }
        SetupLocal = @{ Exit = 0 }
        StartTitel = 'Nakama-Diagnose.flp - FL Studio 2026'
        Pings = (New-Object System.Collections.Generic.List[datetime])
        FlBeiRueckweg = $null
    }
    $script:U = Neue-Testumgebung
    $script:K = Neuer-Kontext @{
        Ticket = 'SELBSTTEST'; Basis = 'basis123'; Head = 'abcdef12'; Repo = $repo; McpRepo = $mcp
        Roh = (Join-Path $wurzel 'roh.md'); LoopMidi = $loopMidi
        PingFristSekunden = 150; InstallFristSekunden = 900; RenderFristSekunden = 600
        Arbeit = (Join-Path $wurzel 'arbeit'); DiagnoseOrdner = (Join-Path $wurzel 'diagnose'); Leise = $true
    }
    New-Item -ItemType Directory -Force -Path $script:K.Arbeit | Out-Null
}

function T-Szenario([string]$name, [hashtable]$kopf = @{}) {
    $inhalt = @{ id = [IO.Path]::GetFileNameWithoutExtension($name); titel = 'Attrappe'; schritte = @() }
    foreach ($schluessel in $kopf.Keys) { $inhalt[$schluessel] = $kopf[$schluessel] }
    [IO.File]::WriteAllText((Join-Path $script:T.Repo "docs\gesundheit\szenarien\$name"), ($inhalt | ConvertTo-Json -Depth 5))
}

function T-Prozess([string]$titel, [string]$mainTitel = $null, [bool]$fenster = $true, [bool]$beendbar = $true) {
    $T = $script:T
    $id = $T.NaechstePid
    $T.NaechstePid += 1
    $haupt = if ($fenster) { @([pscustomobject]@{ Klasse = 'TFruityLoopsMainForm'; Titel = $titel; Sichtbar = $true }) } else { @() }
    $mt = if ($null -ne $mainTitel) { $mainTitel } else { $titel }
    $p = [pscustomobject]@{ Id = $id; Name = 'FL64'; MainWindowTitle = $(if ($fenster) { $mt } else { '' }); MainWindowHandle = [int64]$(if ($fenster) { 1 } else { 0 })
        Fenster = $haupt; Beendbar = $beendbar; Beendet = $false; Render = $false; Start = $T.Uhr; Fertig = $false; ExitCode = $null; Argumente = '' }
    $T.Prozesse.Add($p)
    $T.Gestartet[$id] = $p
    return $id
}

function T-Starte([string]$exe, [string]$argumente) {
    $T = $script:T
    $id = $T.NaechstePid
    $T.NaechstePid += 1
    T-Protokoll "start $argumente"
    if ($argumente -like '/R*') {
        $p = [pscustomobject]@{ Id = $id; Name = 'FL64'; MainWindowTitle = ''; MainWindowHandle = [int64]0; Fenster = @($T.Render.Fenster)
            Beendbar = $true; Beendet = $false; Render = $true; Start = $T.Uhr; Fertig = $false; ExitCode = $T.Render.Exit; Argumente = $argumente }
    } else {
        $p = [pscustomobject]@{ Id = $id; Name = 'FL64'; MainWindowTitle = $T.StartTitel; MainWindowHandle = [int64]1
            Fenster = @([pscustomobject]@{ Klasse = 'TFruityLoopsMainForm'; Titel = $T.StartTitel; Sichtbar = $true })
            Beendbar = $true; Beendet = $false; Render = $false; Start = $T.Uhr; Fertig = $false; ExitCode = $null; Argumente = $argumente }
        if ($null -ne $T.BootNach) { $T.BootMarkeZeit = $T.Uhr.AddSeconds($T.BootNach) }
    }
    $T.Prozesse.Add($p)
    $T.Gestartet[$id] = $p
    return $id
}

function T-Tick {
    $T = $script:T
    if ($T.Render.Haengt) { return }
    foreach ($p in @($T.Prozesse | Where-Object { $_.Render })) {
        # HaengtBei: nur der Render dieses Projekts haengt (P-21, Referenzrender); die Renderdatei traegt den Namen des
        # gerenderten Projekts und seinen Namen als Inhalt.
        if ($T.Render.HaengtBei -and $p.Argumente -like "*$($T.Render.HaengtBei)*") { continue }
        if (($T.Uhr - $p.Start).TotalSeconds -ge $T.Render.Dauer) {
            if ($T.Render.SchreibtDatei -and $p.Argumente -match '/O"([^"]+)" "([^"]+)"') {
                $projekt = [IO.Path]::GetFileName($Matches[2])
                [IO.File]::WriteAllText((Join-Path $Matches[1] ([IO.Path]::ChangeExtension($projekt, '.wav'))), "RENDER-Attrappe $projekt")
            }
            $p.Fertig = $true
            [void]$T.Prozesse.Remove($p)
        }
    }
}

function Neue-Testumgebung {
    $u = @{}
    $u.Jetzt = { $script:T.Uhr }
    $u.Schlafe = { param([double]$s) if ($s -gt 0) { $script:T.Uhr = $script:T.Uhr.AddSeconds($s); T-Tick } }
    $u.Prozesse = { @($script:T.Prozesse | ForEach-Object { [pscustomobject]@{ Id = $_.Id; Name = $_.Name; MainWindowTitle = $_.MainWindowTitle; MainWindowHandle = $_.MainWindowHandle } }) }
    $u.Fenster = { param([int]$id) $p = @($script:T.Prozesse | Where-Object { $_.Id -eq $id }); if ($p.Count -eq 0) { return @() }; @($p[0].Fenster) }
    $u.BeendeProzess = {
        param([int]$id)
        T-Protokoll "beende $id"
        $p = @($script:T.Prozesse | Where-Object { $_.Id -eq $id })
        if ($p.Count -gt 0 -and $p[0].Beendbar) { $p[0].Beendet = $true; [void]$script:T.Prozesse.Remove($p[0]) }
    }
    $u.Git = { param([string[]]$a) T-Protokoll "git $($a -join ' ')"; $script:T.Git }
    $u.Py = { param([string[]]$a) T-Protokoll ("py " + (($a | ForEach-Object { [IO.Path]::GetFileName($_) }) -join ' ')); $script:T.Py }
    $u.AufgabeStarte = {
        param([string]$name)
        $a = $script:T.Aufgaben[$name]
        if (-not $a) { throw "Aufgabe \Nakama\$name fehlt - einmal erhoeht tools/dirigent/install-aufgaben-registrieren.ps1 ausfuehren (NAK-285)" }
        if ($name -eq 'rueckweg') { $script:T.FlBeiRueckweg = $script:T.Prozesse.Count }
        T-Protokoll "aufgabe $name"
    }
    $u.AufgabeZustand = { param([string]$name) $script:T.Aufgaben[$name].Zustand }
    $u.AufgabeErgebnis = { param([string]$name) $script:T.Aufgaben[$name].Exit }
    $u.AufgabeLog = { param([string]$name) @($script:T.Aufgaben[$name].Log) }
    $u.LoopMidiLaeuft = { $script:T.LoopMidiLaeuft }
    $u.StarteLoopMidi = { param([string]$p) T-Protokoll 'loopmidi start'; $script:T.LoopMidiLaeuft = $true }
    $u.Existiert = { param([string]$p) Test-Path -LiteralPath $p }
    $u.Kopiere = { param([string]$von, [string]$nach) Copy-Item -LiteralPath $von -Destination $nach -Force }
    $u.LegeOrdnerAn = { param([string]$p) New-Item -ItemType Directory -Force -Path $p | Out-Null }
    $u.Hash = { param([string]$p) if (Test-Path -LiteralPath $p) { (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash } else { $null } }
    $u.Namen = { param([string]$o) if (Test-Path -LiteralPath $o) { @(Get-ChildItem -LiteralPath $o -File -Force | ForEach-Object { $_.Name }) } else { @() } }
    $u.Loesche = { param([string]$p) T-Protokoll "loesche $([IO.Path]::GetFileName($p))"; [IO.File]::Delete($p) }
    $u.LiesText = { param([string]$p) if (Test-Path -LiteralPath $p) { [IO.File]::ReadAllText($p) } else { $null } }
    $u.SchreibeText = { param([string]$p, [string]$t) [IO.File]::WriteAllText($p, $t) }
    $u.FlPfad = { 'C:\Attrappe\FL64.exe' }
    $u.StarteProzess = { param([string]$exe, [string]$argumente) T-Starte $exe $argumente }
    $u.ProzessLaeuft = { param([int]$id) (@($script:T.Prozesse | Where-Object { $_.Id -eq $id }).Count -gt 0) }
    $u.ProzessExit = { param([int]$id) $p = $script:T.Gestartet[$id]; if ($p -and $p.Fertig -and -not $p.Beendet) { $p.ExitCode } else { $null } }
    $u.BootMarke = {
        $T = $script:T
        if ($T.BootMarkeZeit -and $T.Uhr -ge $T.BootMarkeZeit) {
            return @{ Pfad = 'mcp_boot.json'; Existiert = $true; Zeit = $T.BootMarkeZeit; Inhalt = "{ `"script_version`": `"$($T.BootVersion)`" }"; Version = $T.BootVersion }
        }
        return @{ Pfad = 'mcp_boot.json'; Existiert = $false }
    }
    $u.ControllerInstalliert = { $script:T.ControllerInstalliert }
    $u.SetupLocal = {
        T-Protokoll 'setup-local'
        if ($script:T.SetupLocal.Exit -eq 0) { Copy-Item -LiteralPath (Join-Path $script:T.Mcp 'fl_controller\device_FLStudioMCP.py') -Destination $script:T.ControllerInstalliert -Force }
        @{ Exit = $script:T.SetupLocal.Exit; Zeilen = @('[OK] controller script (Attrappe)') }
    }
    $u.Ping = {
        $script:T.Pings.Add($script:T.Uhr)
        T-Protokoll 'ping'
        if ($script:T.Ping) { & $script:T.Ping } else { @{ Ok = $true; Titel = 'FL Studio 2026'; Version = 'Attrappe'; Port = 'loopMIDI Port 1' } }
    }
    $u.FahreSzenario = {
        param([string]$pfad)
        $name = [IO.Path]::GetFileName($pfad)
        T-Protokoll "szenario $name pid=$($script:K.DiagnosePid)"
        $wirkung = $script:T.Szenarien[$name]
        $code = if ($wirkung -is [scriptblock]) { [int](& $wirkung) } elseif ($null -ne $wirkung) { [int]$wirkung } else { 0 }
        @{ Exit = $code; Zeilen = @("SZENARIO EXIT=$code") }
    }
    return $u
}

function T-Lauf {
    $e = Fahre-Lauf
    return $e
}

function Fahre-Selbsttest([string[]]$nur) {
    $auswahl = @($nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    $faelle = @(Selbsttest-Faelle)
    $rot = 0
    $gelaufen = 0
    foreach ($fall in $faelle) {
        if ($auswahl.Count -gt 0 -and -not ($auswahl -contains $fall.Name -or $auswahl -contains $fall.Zeile)) { continue }
        $gelaufen++
        $script:SammelFehler = New-Object System.Collections.Generic.List[string]
        try { $null = & $fall.Block }
        catch { $script:SammelFehler.Add("Ausnahme: $($_.Exception.Message) (Zeile $($_.InvocationInfo.ScriptLineNumber))") }
        if ($script:SammelFehler.Count -gt 0) {
            $rot++
            Write-Host ("[ROT] {0} {1}: {2}" -f $fall.Zeile, $fall.Name, ($script:SammelFehler -join ' | '))
        } else {
            Write-Host ("[ok] {0} {1}" -f $fall.Zeile, $fall.Name)
        }
    }
    foreach ($w in $script:TestWurzeln) { try { [IO.Directory]::Delete($w, $true) } catch { } }
    if ($gelaufen -eq 0) { Write-Host "SELBSTTEST laufzeit.ps1: kein Fall passt zu '$($auswahl -join ',')'"; return 2 }
    Write-Host ("SELBSTTEST laufzeit.ps1: {0} ok, {1} rot" -f ($gelaufen - $rot), $rot)
    if ($rot -gt 0) { return 4 }
    return 0
}

# ---------------------------------------------------------------- Selbsttest: Faelle
$script:Faelle = New-Object System.Collections.Generic.List[object]
function Fall([string]$zeile, [string]$name, [scriptblock]$block) { $script:Faelle.Add([pscustomobject]@{ Zeile = $zeile; Name = $name; Block = $block }) }
function Selbsttest-Faelle { return $script:Faelle }
function Testfall([string]$name, [string[]]$szenarien = @('bereitschaft.json')) {
    Neuer-Testfall $name
    foreach ($s in $szenarien) { T-Szenario $s }
}

Fall 'M-01' 'lohnt_nicht_ohne_produktpfad' {
    Testfall 'Diff ohne Produktpfad'
    $script:T.Git = @{ Exit = 0; Zeilen = @('docs/beweise/NAK-286.md', 'tools/fl/szenario.py') }
    $e = T-Lauf
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'UEBERSPRUNGEN') "Exit $($e.Code) $($e.Urteil) statt 0 UEBERSPRUNGEN"
    Pruefe ($e.Zusatz -match 'basis123' -and $e.Zusatz -match '\(2 Datei') "Grund nennt Basis und Dateizahl nicht: '$($e.Zusatz)'"
    Pruefe (-not (T-Hat 'aufgabe |start |szenario |py ')) "Schritt nach dem Diff-Entscheid: $($script:T.Protokoll -join '; ')"
}

Fall 'M-02' 'lohnt_vier_faelle' {
    Testfall 'Produktpfad im Diff'
    $l = Lohnt-Es
    Pruefe ($l.Ja -and $l.Grund -match 'Produktpfade im Diff: 1 Datei') "(a) $($l.Ja) '$($l.Grund)'"
    Testfall 'Diff scheitert'
    $script:T.Git = @{ Exit = 128; Zeilen = @('fatal: bad revision') }
    $l = Lohnt-Es
    Pruefe ($l.Ja -and $l.Grund -match 'vorsichtshalber ja') "(b) $($l.Ja) '$($l.Grund)'"
    $e = T-Lauf
    Pruefe ($e.Urteil -ne 'UEBERSPRUNGEN') "(b) der Lauf endet $($e.Urteil) statt weiterzulaufen"
    Testfall 'ohne Basis'
    $script:K.Basis = ''
    $l = Lohnt-Es
    Pruefe ($l.Ja -and $l.Grund -match 'ohne Basis-SHA: immer') "(c) $($l.Ja) '$($l.Grund)'"
    Testfall 'erzwungen'
    $script:K.Erzwingen = $true
    $script:T.Git = @{ Exit = 0; Zeilen = @('docs/beweise/NAK-286.md') }
    $l = Lohnt-Es
    Pruefe ($l.Ja -and $l.Grund -match 'erzwungen') "(d) $($l.Ja) '$($l.Grund)'"
}

Fall 'M-03' 'fremdes_projekt_nie_beenden' {
    Testfall 'fremdes FL vor der Installation'
    $fremd = T-Prozess 'Mein Song.flp - FL Studio 2026'
    $e = T-Lauf
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'UEBERSPRUNGEN' -and $e.Zusatz -match 'Mein Song') "Installation: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
    Pruefe (-not (T-Hat "beende $fremd")) 'Installation: fremdes FL beendet'
    Pruefe (-not (T-Hat 'aufgabe installieren')) 'Installation trotz fremdem FL gestartet'
    Testfall 'fremdes FL vor dem Render' @('nulltest-host.json')
    $script:K.OhneInstallation = $true
    $fremd = T-Prozess 'Mein Song.flp - FL Studio 2026'
    $e = T-Lauf
    # P-20: Traeger der Zusage ist die Renderfreigabe, sichtbar als start /R im Protokoll der Attrappe; Meldung
    # und render.json sind getrennte, nachrangige Pruefungen.
    Pruefe (-not (T-Hat 'start /R')) 'Renderprozess trotz fremdem FL gestartet'
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'UEBERSPRUNGEN' -and $e.Zusatz -match 'vor dem Render') "Render: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
    Pruefe (-not (Test-Path -LiteralPath (Join-Path $script:K.RenderOrdner 'render.json'))) 'Render: render.json trotz fremdem FL geschrieben'
    Pruefe (-not (T-Hat "beende $fremd")) 'Render: fremdes FL beendet'
    Testfall 'fremdes FL vor dem FL-Start'
    $script:K.OhneInstallation = $true
    $fremd = T-Prozess 'Mein Song.flp - FL Studio 2026'
    $e = T-Lauf
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'UEBERSPRUNGEN' -and $e.Zusatz -match "fremdes FL-Projekt offen: 'Mein Song") "FL-Start: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
    Pruefe (-not (T-Hat 'start "')) 'FL-Start trotz fremdem FL'
    Pruefe (-not (T-Hat "beende $fremd")) 'FL-Start: fremdes FL beendet'
}

Fall 'M-04' 'hashen_scheitert_exit3' {
    Testfall '--hashen Exit 1'
    $script:T.Py = @{ Exit = 1; Zeilen = @('FEHLGESCHLAGEN') }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Urteil -eq 'VORAUSSETZUNG' -and $e.Zusatz -match '--hashen Exit 1') "Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
    Pruefe (-not (T-Hat 'aufgabe ')) 'Aufgabe trotz gescheitertem --hashen gestartet'
}

Fall 'M-05' 'installation_verweigert_exit3' {
    Testfall 'installieren Exit 1'
    $script:T.Aufgaben.installieren = @{ Exit = 1; Zustand = 'Ready'; Log = @('ABBRUCH - FL laeuft (Ruhe)') }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'Installation Exit 1') "Exit 1: Exit $($e.Code) '$($e.Zusatz)'"
    Pruefe (-not (T-Hat 'start ')) 'Exit 1: FL trotz verweigerter Installation gestartet'
    Pruefe ((@($script:K.Protokoll) -join "`n") -match 'FL laeuft \(Ruhe\)') 'Exit 1: Logzeilen fehlen im Protokoll'
    Testfall 'installieren Running'
    $script:T.Aufgaben.installieren = @{ Exit = 267009; Zustand = 'Running'; Log = @('laeuft noch') }
    $t0 = $script:T.Uhr
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'Running') "Running: Exit $($e.Code) '$($e.Zusatz)'"
    Pruefe (($script:T.Uhr - $t0).TotalSeconds -ge 900) "Running: Frist 900 s nicht abgewartet ($(($script:T.Uhr - $t0).TotalSeconds) s auf der Uhr-Attrappe)"
    Testfall 'Aufgabe fehlt'
    $script:T.Aufgaben.Remove('installieren')
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'install-aufgaben-registrieren\.ps1') "fehlt: Exit $($e.Code) '$($e.Zusatz)'"
}

Fall 'M-06' 'pruefen_nicht_aktuell_exit3' {
    Testfall 'pruefen Exit 1'
    $script:T.Aufgaben.pruefen = @{ Exit = 1; Zustand = 'Ready'; Log = @('Installierter Stand gegen das Manifest:', '  aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3') }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Urteil -eq 'VORAUSSETZUNG' -and $e.Zusatz -match 'nicht aktuell') "pruefen Exit 1: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)' statt 3 VORAUSSETZUNG 'installiert, aber nicht aktuell'"
    Pruefe (-not (T-Hat 'start "')) 'pruefen Exit 1: FL-Start gegen einen ungeprueften Bau'
    Testfall 'pruefen ohne aktuell je Artefakt'
    $script:T.Aufgaben.pruefen = @{ Exit = 0; Zustand = 'Ready'; Log = @('Installierter Stand gegen das Manifest:', '  aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3', '  nicht installiert C:\Program Files\evenacadia\Nakama\eqcop-broker.exe') }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'nicht aktuell') "nicht installiert: Exit $($e.Code) '$($e.Zusatz)'"
}

Fall 'NAK-297' 'titel_ohne_haupttitel' {
    Testfall 'Diagnose-FL mit leerem MainWindowTitle'
    $diagnose = T-Prozess 'Nakama-Diagnose.flp - FL Studio 2026' ''
    $e = T-Lauf
    Pruefe (-not ($e.Urteil -eq 'UEBERSPRUNGEN' -and $e.Zusatz -match 'fremdes')) "leerer MainWindowTitle als Fremdprojekt gewertet: $($e.Urteil) '$($e.Zusatz)'"
    Pruefe (T-Hat "beende $diagnose") 'das eigene Diagnose-FL (Titel im Hauptfenster) nicht beendet'
    Testfall 'FL-Fenster ohne lesbaren Titel'
    $ohne = T-Prozess '' ''
    $e = T-Lauf
    Pruefe (-not ($e.Zusatz -match "fremdes FL-Projekt offen: ''")) "leerer Titel als fremdes Projekt: '$($e.Zusatz)'"
    Pruefe (-not (T-Hat "beende $ohne")) 'FL ohne lesbaren Titel beendet'
}

Fall 'M-07' 'loopmidi_und_restprozesse' {
    Testfall 'loopMIDI vor FL'
    $script:T.LoopMidiLaeuft = $false
    $null = T-Lauf
    $iLoop = T-Index 'loopmidi start'
    $iStart = T-Index 'start "'
    Pruefe ($iLoop -ge 0 -and $iStart -gt $iLoop) "Reihenfolge loopMIDI ($iLoop) vor FL-Start ($iStart) verletzt"
    Testfall 'loopMIDI fehlt'
    $script:T.LoopMidiLaeuft = $false
    [IO.File]::Delete($script:K.LoopMidi)
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'loopMIDI fehlt') "fehlt: Exit $($e.Code) '$($e.Zusatz)'"
    Testfall 'Restprozesse'
    $script:K.OhneInstallation = $true
    $rest1 = T-Prozess '' '' $false $true
    $rest2 = T-Prozess '' '' $false $false
    $e = T-Lauf
    Pruefe (T-Hat "beende $rest1") 'beendbarer Restprozess nicht beendet'
    Pruefe ((@($script:K.Protokoll) -join "`n") -match "FL-Restprozess PID ${rest2}: nicht beendbar") 'nicht beendbarer Restprozess nicht geloggt'
    Pruefe ($e.Code -eq 0) "Restprozesse: Lauf endet Exit $($e.Code) statt weiterzulaufen"
}

Fall 'M-08' 'ping_frist_exit3' {
    Testfall 'kein Ping'
    $script:T.Ping = { @{ Ok = $false; Fehler = 'Timeout waiting for FL Studio response after 3.0s' } }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'kein Ping binnen 150 s' -and $e.Zusatz -match 'Boot-Marke') "Exit $($e.Code) '$($e.Zusatz)'"
    Pruefe ($script:T.Pings.Count -eq 30) "Ping-Versuche $($script:T.Pings.Count) statt 30 (Takt 5 s, Frist 150 s)"
}

Fall 'M-08' 'schonfrist_vor_trigger' {
    Testfall 'Schonfrist'
    $script:T.BootNach = 5
    $null = T-Lauf
    Pruefe ($script:T.Pings.Count -gt 0) 'kein Ping gesendet'
    if ($script:T.Pings.Count -gt 0) {
        $abstand = ($script:T.Pings[0] - $script:T.BootMarkeZeit).TotalSeconds
        Pruefe ($abstand -ge 8) "erster Trigger $abstand s nach der Boot-Marke statt mindestens 8 s"
    }
}

function T-LetzterIndex([string]$muster) {
    for ($i = $script:T.Protokoll.Count - 1; $i -ge 0; $i--) { if ($script:T.Protokoll[$i] -match $muster) { return $i } }
    return -1
}
function K-Index([string]$muster) {
    for ($i = 0; $i -lt $script:K.Protokoll.Count; $i++) { if ($script:K.Protokoll[$i] -match $muster) { return $i } }
    return -1
}
$script:NulltestWirkung = {
    [IO.File]::WriteAllText((Join-Path $script:K.RenderOrdner 'ergebnis.json'),
        ('{"format":"nakama.laufzeit.nulltest.v1","urteil":"' + $script:T.NulltestUrteil + '","v":0,"g":1.0}'))
    $script:T.NulltestCode
}
$script:FuenfSzenarien = @('bereitschaft.json', 'fenster.json', 'nulltest-host.json', 'snapshot-runde01.json', 'u40-aktivitaetsgate.json')
$script:EchteSzenarien = Join-Path $Repo 'docs\gesundheit\szenarien'

Fall 'M-09' 'controller_veraltet_neu_installieren' {
    Testfall 'Controller veraltet'
    [IO.File]::WriteAllText($script:T.ControllerInstalliert, "SCRIPT_VERSION = `"2026-09-12`"`n")
    $e = T-Lauf
    Pruefe (T-Hat 'setup-local') 'veralteter Controller: setup-local.ps1 nicht gefahren'
    Pruefe ((@($script:K.Protokoll) -join "`n") -match 'Controller-Skript: SHA-256 Repo [0-9A-F]{64}, installiert [0-9A-F]{64}') 'veralteter Controller: keine Rohzeile mit beiden Hashes'
    Pruefe ($e.Code -eq 0) "veralteter Controller: Exit $($e.Code) '$($e.Zusatz)'"
    Testfall 'Controller gleich'
    $e = T-Lauf
    Pruefe (-not (T-Hat 'setup-local')) 'gleicher Controller: setup-local.ps1 trotzdem gefahren'
    Pruefe ($e.Code -eq 0) "gleicher Controller: Exit $($e.Code) '$($e.Zusatz)'"
    Testfall 'Diagnose-FL laeuft, Controller veraltet'
    $script:K.OhneInstallation = $true
    [IO.File]::WriteAllText($script:T.ControllerInstalliert, 'alt')
    $diagnose = T-Prozess 'Nakama-Diagnose.flp - FL Studio 2026'
    $null = T-Lauf
    $iBeende = T-Index "beende $diagnose"
    $iSetup = T-Index 'setup-local'
    $iStart = T-Index 'start "'
    Pruefe ($iBeende -ge 0 -and $iSetup -ge 0 -and $iStart -gt $iSetup) "Neustart nach setup-local.ps1 fehlt (beende $iBeende, setup $iSetup, start $iStart)"
    Testfall 'setup-local scheitert'
    [IO.File]::WriteAllText($script:T.ControllerInstalliert, 'alt')
    $script:T.SetupLocal = @{ Exit = 1 }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'setup-local') "setup-local Exit 1: Exit $($e.Code) '$($e.Zusatz)'"
    Testfall 'keine neue Boot-Marke'
    [IO.File]::WriteAllText($script:T.ControllerInstalliert, 'alt')
    $script:T.BootVersion = '2026-09-12'
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'script_version') "alte Boot-Marke: Exit $($e.Code) '$($e.Zusatz)'"
}

Fall 'M-10' 'szenario_exitcodes' {
    foreach ($f in @(@{ C = 0; U = 'GEMESSEN'; E = 0 }, @{ C = 2; U = 'VERFEHLT'; E = 4 }, @{ C = 4; U = 'VERFEHLT'; E = 4 },
                     @{ C = 5; U = 'VORAUSSETZUNG'; E = 3 }, @{ C = 3; U = 'VORAUSSETZUNG'; E = 3 })) {
        Testfall "Szenario-Exit $($f.C)" @('a.json', 'b.json')
        $script:T.Szenarien['a.json'] = $f.C
        $e = T-Lauf
        Pruefe ($e.Code -eq $f.E -and $e.Urteil -eq $f.U) "Szenario-Exit $($f.C): Runner-Exit $($e.Code) $($e.Urteil) statt $($f.E) $($f.U)"
        $weiter = T-Hat 'szenario b\.json'
        if ($f.C -eq 3) { Pruefe (-not $weiter) 'Szenario-Exit 3: die Folge lief weiter' }
        else { Pruefe $weiter "Szenario-Exit $($f.C): die Folge brach ab" }
    }
}

Fall 'M-10' 'kette_erreicht_folgeszenarien' {
    Testfall 'KETTE an Position 3' $script:FuenfSzenarien
    $script:T.NulltestUrteil = 'KETTE'
    $script:T.NulltestCode = 5
    $script:T.Szenarien['nulltest-host.json'] = $script:NulltestWirkung
    $e = T-Lauf
    foreach ($n in $script:FuenfSzenarien) { Pruefe (T-Hat "szenario $([regex]::Escape($n))") "$n lief nicht" }
    Pruefe ($e.Code -eq 3 -and $e.Urteil -eq 'VORAUSSETZUNG') "Gesamturteil $($e.Code) $($e.Urteil) statt 3 VORAUSSETZUNG"
    Pruefe (-not (T-Hat 'aufgabe rueckweg')) 'Rueckweg nach KETTE'
}

Fall 'M-11' 'kopfzeile_erste_fassung' {
    foreach ($f in @(@{ Name = 'gemessen'; Git = @{ Exit = 0; Zeilen = @('broker/src/lib.rs') }; U = 'GEMESSEN' },
                     @{ Name = 'uebersprungen'; Git = @{ Exit = 0; Zeilen = @('docs/x.md') }; U = 'UEBERSPRUNGEN' })) {
        Testfall "Kopfzeile $($f.Name)"
        $script:T.Git = $f.Git
        $e = T-Lauf
        $zeile = $script:K.Kopfzeilen[$script:K.Kopfzeilen.Count - 1]
        Pruefe ($zeile -match '^LAUFZEIT SELBSTTEST abcdef12 (GEMESSEN|UEBERSPRUNGEN|VORAUSSETZUNG|VERFEHLT) .* roh=.+$') "$($f.Name): Kopfzeile ausser Form: '$zeile'"
        Pruefe ($e.Urteil -eq $f.U -and $zeile -match " $($f.U) ") "$($f.Name): Urteilswort '$($e.Urteil)' statt $($f.U)"
        $roh = [IO.File]::ReadAllText($script:K.Roh)
        Pruefe ($roh -match "(?m)^# Laufzeit-Arm SELBSTTEST - $($f.U)" -and $roh -match "(?m)^Kopfzeile: LAUFZEIT SELBSTTEST abcdef12 $($f.U)" -and $roh -match '(?m)^## Protokoll') "$($f.Name): Rohdatei ohne Kopf, Kopfzeile oder Protokoll"
        if ($f.U -eq 'GEMESSEN') { Pruefe ($roh -match '(?m)^## Szenarien') 'gemessen: Rohdatei ohne ## Szenarien' }
    }
}

Fall 'M-11' 'kopfzeile_und_protokoll' {
    # M-11 und F-24 an der Zeile: Szenario-Exit 5 steht in der Liste der Szenarioexits der Kopfzeile, nie als
    # Runner-Exit; Render- und Briefkastenschritt stehen unter ## Protokoll der Rohdatei.
    Testfall 'KETTE mit Render' $script:FuenfSzenarien
    $script:T.NulltestUrteil = 'KETTE'
    $script:T.NulltestCode = 5
    $script:T.Szenarien['nulltest-host.json'] = $script:NulltestWirkung
    $e = T-Lauf
    $zeile = $script:K.Kopfzeilen[$script:K.Kopfzeilen.Count - 1]
    Pruefe ($e.Code -eq 3 -and $zeile -match '^LAUFZEIT SELBSTTEST abcdef12 VORAUSSETZUNG .*szenarien=5 verfehlt=1 \[[^\]]*nulltest-host\.json=5[^\]]*\]') "Kopfzeile ohne Szenario-Exit 5 in der Liste der Szenarioexits: Exit $($e.Code) '$zeile'"
    $roh = [IO.File]::ReadAllText($script:K.Roh)
    $protokoll = if ($roh -match '(?s)## Protokoll\s+```text\r?\n(.*?)```') { $Matches[1] } else { '' }
    Pruefe ($protokoll -match '(?m)^\[[^\]]+\] Render: Exit 0, Dauer ') 'Rohdatei: Renderschritt nicht unter ## Protokoll'
    Pruefe ($protokoll -match '(?m)^\[[^\]]+\] Briefkasten: .* bereit, ') 'Rohdatei: Briefkastenschritt nicht unter ## Protokoll'
}

Fall 'M-12' 'projekt_unveraendert' {
    Testfall 'Arbeitskopie veraendert'
    $script:T.Szenarien['bereitschaft.json'] = { [IO.File]::AppendAllText((Join-Path $script:K.ProjektOrdner 'Nakama-Diagnose.flp'), 'gespeichert'); 0 }
    $e = T-Lauf
    Pruefe ($e.Code -eq 4 -and $e.Urteil -eq 'VERFEHLT' -and $e.Zusatz -match 'Diagnoseprojekt veraendert') "veraenderte Arbeitskopie: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)' statt 4 VERFEHLT 'Diagnoseprojekt veraendert'"
    Pruefe ((@($script:K.Protokoll) -join "`n") -match 'Diagnoseprojekt: SHA-256 Repo [0-9A-F]{64}, Arbeitskopie [0-9A-F]{64}') 'keine Rohzeile mit den Projekt-Hashes'
    Testfall 'Arbeitskopie unveraendert'
    $e = T-Lauf
    Pruefe ($e.Code -eq 0 -and $e.Zusatz -notmatch 'veraendert') "unveraendert: Exit $($e.Code) '$($e.Zusatz)'"
}

Fall 'M-64' 'referenzprojekte_aus_szenariodatei' {
    # P-18 (§38.2): die Namen der Referenzprojekte stehen in der Szenariodatei nulltest-host.json des Repos - genau die
    # Namen der Karte U43, im Ordner des Diagnoseprojekts. Der Runner kopiert sie wie das Diagnoseprojekt in seine
    # Arbeitskopie und vergleicht die SHA-256 am Anfang und am Ende (M-12 fuer alle drei Projekte). Kein anderer Name,
    # kein Suchen nach Mustern: Attrappen mit den frueheren und aehnlichen Namen liegen daneben.
    $karte = @('Nakama-Diagnose-Referenz.flp', 'Nakama-Diagnose-Verarbeitung.flp')
    $koeder = @('Nakama-Diagnose-ohne-Slots.flp', 'Nakama-Diagnose-Verarbeitung-ein.flp', 'Nakama-Diagnose-Referenz-alt.flp')
    $lege = {
        param([string[]]$namen)
        [IO.File]::WriteAllText((Join-Path $script:T.Repo 'docs\gesundheit\szenarien\nulltest-host.json'),
            [IO.File]::ReadAllText((Join-Path $script:EchteSzenarien 'nulltest-host.json')))
        foreach ($n in $namen) { [IO.File]::WriteAllText((Join-Path $script:T.Repo "eq-copilot\fixtures\fl\$n"), "FLP-Attrappe $n") }
    }
    $kopien = { (@(Get-ChildItem -LiteralPath $script:K.ProjektOrdner -File | ForEach-Object { $_.Name }) | Sort-Object) -join ', ' }

    Testfall 'Projekte der Karte neben Koedern' @('nulltest-host.json')
    & $lege ($karte + $koeder)
    $e = T-Lauf
    $soll = (@(@('Nakama-Diagnose.flp') + $karte) | Sort-Object) -join ', '
    Pruefe ((& $kopien) -eq $soll) "Arbeitskopie traegt [$(& $kopien)] statt [$soll] (Namen der Karte U43 aus nulltest-host.json, kein Muster)"
    $protokoll = @($script:K.Protokoll) -join "`n"
    foreach ($n in $karte) {
        $h = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $script:T.Repo "eq-copilot\fixtures\fl\$n")).Hash
        $m = [regex]::Escape($n)
        Pruefe ($protokoll -match "\] Referenzprojekt ${m}: SHA-256 Repo $h, Arbeitskopie $h") "Referenzprojekt $n ohne SHA-256 von Repo und Arbeitskopie am Anfang"
        Pruefe ($protokoll -match "\] Referenzprojekt $m am Ende: SHA-256 Repo $h, Arbeitskopie $h") "Referenzprojekt $n ohne SHA-256 von Repo und Arbeitskopie am Ende"
    }
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'GEMESSEN') "unveraenderte Referenzprojekte: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"

    Testfall 'Referenzprojekte waehrend des Laufs veraendert' @('nulltest-host.json')
    & $lege $karte
    $script:T.Szenarien['nulltest-host.json'] = {
        [IO.File]::AppendAllText((Join-Path $script:K.ProjektOrdner 'Nakama-Diagnose-Verarbeitung.flp'), 'gespeichert')
        [IO.File]::AppendAllText((Join-Path $script:T.Repo 'eq-copilot\fixtures\fl\Nakama-Diagnose-Referenz.flp'), 'gespeichert')
        0
    }
    $e = T-Lauf
    Pruefe ($e.Code -eq 4 -and $e.Urteil -eq 'VERFEHLT' -and $e.Zusatz -match 'Referenzprojekt Nakama-Diagnose-Verarbeitung\.flp veraendert' -and $e.Zusatz -match 'Referenzprojekt Nakama-Diagnose-Referenz\.flp veraendert') "veraenderte Referenzprojekte (Arbeitskopie, Repo): Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)' statt 4 VERFEHLT mit beiden Namen"

    Testfall 'ohne die Projekte der Karte' @('nulltest-host.json')
    & $lege $koeder
    $e = T-Lauf
    Pruefe ((& $kopien) -eq 'Nakama-Diagnose.flp') "ohne die Projekte der Karte: Arbeitskopie traegt [$(& $kopien)]"
    $protokoll = @($script:K.Protokoll) -join "`n"
    foreach ($n in $karte) {
        Pruefe ($protokoll -match "Referenzprojekt $([regex]::Escape($n)): fehlt neben dem Diagnoseprojekt") "ohne die Projekte der Karte: keine Zeile 'Referenzprojekt ${n}: fehlt neben dem Diagnoseprojekt'"
    }
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'GEMESSEN') "ohne die Projekte der Karte: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
}

Fall 'M-64' 'referenzrender_vor_fl_start' {
    # P-21 (a) (§44.3): vor dem Start des Diagnose-FL rendert der Runner jedes kopierte Referenzprojekt wie das
    # Diagnoseprojekt (F-18) aus der Arbeitskopie in einen eigenen Ordner render\referenz\<Name>\ mit eigenem render.json
    # (Format nakama.laufzeit.render.v1, Feld projekt) und eigener Renderdatei; Ort und Name des Auslieferungsrenders,
    # seines render.json und seiner ergebnis.json bleiben (F-26). Ein fehlendes Referenzprojekt bekommt einen Renderstatus
    # mit Grund und keinen Render; die Reihenfolge der Renders steht im Protokoll.
    $karte = @('Nakama-Diagnose-Verarbeitung.flp', 'Nakama-Diagnose-Referenz.flp')
    $fehlt = 'Referenzprojekt fehlt (Karte U43, K-286-1)'
    $lege = {
        param([string[]]$namen)
        [IO.File]::WriteAllText((Join-Path $script:T.Repo 'docs\gesundheit\szenarien\nulltest-host.json'),
            [IO.File]::ReadAllText((Join-Path $script:EchteSzenarien 'nulltest-host.json')))
        foreach ($n in $namen) { [IO.File]::WriteAllText((Join-Path $script:T.Repo "eq-copilot\fixtures\fl\$n"), "FLP-Attrappe $n") }
    }
    $status = {
        param([string]$ordner)
        $datei = Join-Path $ordner 'render.json'
        if (Test-Path -LiteralPath $datei) { [IO.File]::ReadAllText($datei) | ConvertFrom-Json } else { $null }
    }
    $renders = { @($script:T.Protokoll | Where-Object { $_ -match ' start /R ' }) }

    Testfall 'drei Renders vor dem FL-Start' @('nulltest-host.json')
    & $lege $karte
    $e = T-Lauf
    $iStart = T-Index 'start "'
    Pruefe ((& $renders).Count -eq 3) "Renders vor dem FL-Start: $((& $renders).Count) statt 3 (Diagnoseprojekt und die zwei Referenzprojekte der Karte)"
    $soll = @([pscustomobject]@{ Name = 'Nakama-Diagnose.flp'; Ordner = $script:K.RenderOrdner }) +
        @($karte | ForEach-Object { [pscustomobject]@{ Name = $_; Ordner = (Join-Path $script:K.RenderOrdner "referenz\$_") } })
    $voriger = -1
    for ($i = 0; $i -lt $soll.Count; $i++) {
        $s = $soll[$i]
        $idx = T-Index ([regex]::Escape(' start /R /Ewav /O"' + $s.Ordner + '" "' + (Join-Path $script:K.ProjektOrdner $s.Name) + '"'))
        Pruefe ($idx -ge 0) "Render $($i + 1) $($s.Name) nicht aus der Arbeitskopie in $($s.Ordner) gestartet"
        Pruefe ($idx -lt 0 -or $idx -lt $iStart) "Render $($i + 1) $($s.Name) nach dem FL-Start (Index $idx, FL-Start $iStart)"
        Pruefe ($idx -lt 0 -or $idx -gt $voriger) "Render $($i + 1) $($s.Name) nicht nach Render $i (Reihenfolge der Szenariodatei)"
        if ($idx -ge 0) { $voriger = $idx }
    }
    $a = & $status $script:K.RenderOrdner
    $wavA = Join-Path $script:K.RenderOrdner 'Nakama-Diagnose.wav'
    Pruefe ($null -ne $a -and $a.projekt -eq (Join-Path $script:K.ProjektOrdner 'Nakama-Diagnose.flp') -and $a.datei -eq $wavA -and -not $a.grund) "render.json des Auslieferungsrenders nennt projekt '$($a.projekt)', datei '$($a.datei)', grund '$($a.grund)' statt Nakama-Diagnose.flp und $wavA"
    $inhaltA = if (Test-Path -LiteralPath $wavA) { [IO.File]::ReadAllText($wavA) } else { '(fehlt)' }
    Pruefe ($inhaltA -eq 'RENDER-Attrappe Nakama-Diagnose.flp') "Renderdatei des Auslieferungsrenders traegt '$inhaltA' statt den Render von Nakama-Diagnose.flp"
    foreach ($n in $karte) {
        $ordner = Join-Path $script:K.RenderOrdner "referenz\$n"
        $r = & $status $ordner
        $wav = Join-Path $ordner ([IO.Path]::ChangeExtension($n, '.wav'))
        $kopie = Join-Path $script:K.ProjektOrdner $n
        $hash = if (Test-Path -LiteralPath $kopie) { (Get-FileHash -Algorithm SHA256 -LiteralPath $kopie).Hash } else { '(keine Arbeitskopie)' }
        Pruefe ($null -ne $r -and $r.format -eq 'nakama.laufzeit.render.v1' -and $r.projekt -eq $kopie -and $r.datei -eq $wav -and -not $r.grund -and $r.exit -eq 0) "Renderstatus ${n} in $ordner fehlt oder nennt projekt '$($r.projekt)', datei '$($r.datei)', grund '$($r.grund)', exit '$($r.exit)'"
        Pruefe ($null -ne $r -and $r.sha256_projekt -eq $hash) "Renderstatus ${n} ohne SHA-256 der Arbeitskopie ('$($r.sha256_projekt)' statt $hash)"
        $inhalt = if (Test-Path -LiteralPath $wav) { [IO.File]::ReadAllText($wav) } else { '(fehlt)' }
        Pruefe ($inhalt -eq "RENDER-Attrappe $n") "Renderdatei ${n} in ihrem Ordner traegt '$inhalt'"
    }
    Pruefe ((K-Index 'Renderfolge vor dem FL-Start: 1 Nakama-Diagnose\.flp .*, 2 Nakama-Diagnose-Verarbeitung\.flp .*, 3 Nakama-Diagnose-Referenz\.flp') -ge 0) 'Protokoll ohne Renderfolge vor dem FL-Start (1 Diagnoseprojekt, 2 Verarbeitung, 3 Referenz)'
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'GEMESSEN') "drei Renders: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"

    Testfall 'Referenzprojekte fehlen' @('nulltest-host.json')
    & $lege @()
    $alt = Join-Path $script:K.RenderOrdner 'referenz\Nakama-Diagnose-Referenz.flp'
    New-Item -ItemType Directory -Force -Path $alt | Out-Null
    [IO.File]::WriteAllText((Join-Path $alt 'render.json'), '{"format":"nakama.laufzeit.render.v1","datei":"frueher.wav","grund":null}')
    [IO.File]::WriteAllText((Join-Path $alt 'Nakama-Diagnose-Referenz.wav'), 'RENDER eines frueheren Laufs')
    $e = T-Lauf
    Pruefe ((& $renders).Count -eq 1) "fehlende Referenzprojekte: $((& $renders).Count) Renders statt 1 (nur das Diagnoseprojekt)"
    $iFl = K-Index 'FL gestartet: PID'
    foreach ($n in $karte) {
        $ordner = Join-Path $script:K.RenderOrdner "referenz\$n"
        $r = & $status $ordner
        Pruefe ($null -ne $r -and $r.format -eq 'nakama.laufzeit.render.v1' -and $r.grund -eq $fehlt -and -not $r.datei -and $r.projekt -eq (Join-Path $script:K.ProjektOrdner $n)) "fehlendes Referenzprojekt ${n}: Renderstatus fehlt oder nennt grund '$($r.grund)', datei '$($r.datei)', projekt '$($r.projekt)' statt '$fehlt' ohne Datei"
        Pruefe (-not (Test-Path -LiteralPath (Join-Path $ordner ([IO.Path]::ChangeExtension($n, '.wav'))))) "fehlendes Referenzprojekt ${n}: Renderdatei eines frueheren Laufs liegt noch"
        $iKein = K-Index ([regex]::Escape("Referenzrender ${n}: kein Render - $fehlt"))
        Pruefe ($iKein -ge 0 -and $iKein -lt $iFl) "fehlendes Referenzprojekt ${n}: keine Protokollzeile 'kein Render' vor dem FL-Start (Zeile $iKein, FL-Start $iFl)"
    }
    Pruefe ($e.Code -eq 0) "fehlende Referenzprojekte: Exit $($e.Code) '$($e.Zusatz)'"

    Testfall 'Referenzrender haengt' @('nulltest-host.json')
    & $lege $karte
    $script:T.Render.HaengtBei = 'Nakama-Diagnose-Verarbeitung.flp'
    $e = T-Lauf
    $haengend = @($script:T.Gestartet.Values | Where-Object { $_.Render -and $_.Argumente -like '*Nakama-Diagnose-Verarbeitung.flp*' })
    $beendet = @($script:T.Protokoll | Where-Object { $_ -match ' beende ' })
    Pruefe ($haengend.Count -eq 1 -and $beendet.Count -eq 1 -and (T-Hat "beende $($haengend[0].Id)$")) "haengender Referenzrender: nicht genau seine PID beendet ($($beendet -join '; '))"
    $r = & $status (Join-Path $script:K.RenderOrdner 'referenz\Nakama-Diagnose-Verarbeitung.flp')
    Pruefe ($null -ne $r -and $r.grund -match 'haengt' -and -not $r.datei -and $r.dauer_s -ge 600) "haengender Referenzrender: Renderstatus grund '$($r.grund)', Dauer $($r.dauer_s) s statt Grund 'haengt' nach 600 s"
    $weiter = & $status (Join-Path $script:K.RenderOrdner 'referenz\Nakama-Diagnose-Referenz.flp')
    $iWeiter = T-Index ([regex]::Escape('\Nakama-Diagnose-Referenz.flp"'))
    Pruefe ($null -ne $weiter -and $weiter.datei -and $iWeiter -ge 0 -and $iWeiter -lt (T-Index 'start "')) 'nach dem haengenden Referenzrender lief der naechste Render nicht vor dem FL-Start'
}

Fall 'M-13' 'volllauf_erzwingen_beenden' {
    Testfall 'Volllauf'
    $script:K.Erzwingen = $true
    $script:K.Beenden = $true
    $script:T.Git = @{ Exit = 0; Zeilen = @('docs/x.md') }
    $script:T.Szenarien['bereitschaft.json'] = { $script:T.FremdPid = T-Prozess 'Mein Song.flp - FL Studio 2026'; 0 }
    $e = T-Lauf
    Pruefe (T-Hat 'szenario bereitschaft\.json') 'Szenario lief trotz -Erzwingen nicht'
    Pruefe (-not (T-Hat 'git ')) 'Diff-Entscheid trotz -Erzwingen'
    Pruefe (-not (T-Hat "beende $($script:T.FremdPid)")) '-Beenden hat das fremde FL beendet'
    $diagnose = @($script:T.Gestartet.Values | Where-Object { -not $_.Render -and $_.MainWindowTitle -like '*Nakama-Diagnose*' })
    Pruefe ($diagnose.Count -gt 0 -and $diagnose[-1].Beendet) '-Beenden hat das Diagnose-FL nicht beendet'
    Pruefe ($e.Code -eq 0) "Volllauf: Exit $($e.Code) '$($e.Zusatz)'"
}

Fall 'M-55' 'anfrage_lebenszyklus' {
    Testfall 'Anfrage und simulierter Exit 3'
    $d = $script:K.DiagnoseOrdner
    $a = Join-Path $d 'antwort'
    New-Item -ItemType Directory -Force -Path $a | Out-Null
    [IO.File]::WriteAllText((Join-Path $d 'anfrage.json'), '{"format":"nakama.diagnose.anfrage.v1","anfrage_id":"00000000000000000000000000000001"}')
    $alt = '00000000000000000000000000000001.gen.4242.0123456789abcdef.0123456789abcdef0123456789abcdef.json'
    [IO.File]::WriteAllText((Join-Path $a $alt), '{}')
    [IO.File]::WriteAllText((Join-Path $a ($alt + '.tmp-4242')), '')
    [IO.File]::WriteAllText((Join-Path $a 'notiz.txt'), 'fremd')
    $script:T.Szenarien['bereitschaft.json'] = {
        [IO.File]::WriteAllText((Join-Path $script:K.DiagnoseOrdner 'anfrage.json'), '{"format":"nakama.diagnose.anfrage.v1","anfrage_id":"00000000000000000000000000000002"}')
        3
    }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3) "simulierter Exit 3: Runner-Exit $($e.Code)"
    Pruefe (-not (Test-Path -LiteralPath (Join-Path $d 'anfrage.json'))) 'anfrage.json liegt nach dem Ausgang'
    $iAnfrage = T-Index 'loesche anfrage\.json'
    $iAntwort = T-Index "loesche $([regex]::Escape($alt))$"
    Pruefe ($iAnfrage -ge 0 -and $iAntwort -gt $iAnfrage) "Aufraeumen: Anfrage ($iAnfrage) vor den Antworten ($iAntwort) verletzt"
    Pruefe (T-Hat 'loesche .*\.json\.tmp-4242') 'Temp-Antwort frueherer Laeufe nicht entfernt'
    Pruefe (Test-Path -LiteralPath (Join-Path $a 'notiz.txt')) 'fremde Datei im Antwortordner entfernt'
}

Fall 'M-59' 'frischer_start_schluessel' {
    Testfall 'mit frischer_start' @('a.json')
    T-Szenario 'b.json' @{ frischer_start = $true }
    $script:K.OhneInstallation = $true
    $null = T-Lauf
    $iA = T-Index 'szenario a\.json'
    $iB = T-Index 'szenario b\.json'
    $zwischen = @(($iA + 1)..([math]::Max($iA + 1, $iB - 1)) | Where-Object { $_ -lt $iB } | ForEach-Object { $script:T.Protokoll[$_] })
    Pruefe (@($zwischen | Where-Object { $_ -match 'start "' }).Count -eq 1 -and @($zwischen | Where-Object { $_ -match ' beende ' }).Count -ge 1) "frischer_start: kein Beenden und Neustart zwischen a und b ($($zwischen -join '; '))"
    Testfall 'ohne frischer_start' @('a.json', 'b.json')
    $script:K.OhneInstallation = $true
    $null = T-Lauf
    Pruefe (@($script:T.Protokoll | Where-Object { $_ -match 'start "' }).Count -eq 1) 'ohne Schluessel: Neustart'
    Testfall 'Neustart scheitert' @('a.json')
    T-Szenario 'b.json' @{ frischer_start = $true }
    $script:K.OhneInstallation = $true
    $script:T.Szenarien['a.json'] = { $script:T.Ping = { @{ Ok = $false; Fehler = 'Timeout' } }; 0 }
    $e = T-Lauf
    Pruefe ($e.Code -eq 3 -and $e.Zusatz -match 'frischer_start') "gescheiterter Neustart: Exit $($e.Code) '$($e.Zusatz)'"
    Pruefe (-not (T-Hat 'szenario b\.json')) 'Szenario lief trotz gescheitertem Neustart'
}

Fall 'M-60' 'render_ablauf' {
    Testfall 'Render mit Datei' @('nulltest-host.json')
    $null = T-Lauf
    $status = [IO.File]::ReadAllText((Join-Path $script:K.RenderOrdner 'render.json')) | ConvertFrom-Json
    Pruefe ($status.datei -and -not $status.grund -and $status.exit -eq 0) "mit Datei: render.json $($status | ConvertTo-Json -Compress)"
    $iRender = T-Index 'start /R'
    $iStart = T-Index 'start "'
    Pruefe ($iRender -ge 0 -and $iStart -gt $iRender) "Render ($iRender) nicht vor dem FL-Start ($iStart)"
    Pruefe (T-Hat 'start /R /Ewav /O"') 'Aufruf nicht nach dem FL-Handbuch (/R /Ewav /O)'
    Testfall 'Render ohne Datei' @('nulltest-host.json')
    $script:T.Render.SchreibtDatei = $false
    $null = T-Lauf
    $status = [IO.File]::ReadAllText((Join-Path $script:K.RenderOrdner 'render.json')) | ConvertFrom-Json
    Pruefe (-not $status.datei -and $status.grund -match 'keine Datei') "ohne Datei: Grund '$($status.grund)'"
    Testfall 'Render haengt' @('nulltest-host.json')
    $script:T.Render.Haengt = $true
    $script:T.Render.Fenster = @([pscustomobject]@{ Klasse = 'TMsgForm'; Titel = 'Cloud path warning'; Sichtbar = $true })
    $null = T-Lauf
    $render = @($script:T.Gestartet.Values | Where-Object { $_.Render })
    $status = [IO.File]::ReadAllText((Join-Path $script:K.RenderOrdner 'render.json')) | ConvertFrom-Json
    Pruefe ($status.grund -match 'haengt' -and $status.grund -match 'Cloud path warning') "haengt: Grund '$($status.grund)'"
    Pruefe ($render.Count -eq 1 -and (T-Hat "beende $($render[0].Id)") -and @($script:T.Protokoll | Where-Object { $_ -match ' beende ' }).Count -eq 1) 'haengt: nicht genau die Render-PID beendet'
    Pruefe ($status.dauer_s -ge 600) "haengt: Frist 600 s nicht abgewartet ($($status.dauer_s) s)"
}

Fall 'M-65' 'rueckweg_nur_bei_abweichung' {
    Testfall 'ABWEICHUNG' $script:FuenfSzenarien
    $script:T.NulltestUrteil = 'ABWEICHUNG'
    $script:T.NulltestCode = 4
    $script:T.Szenarien['nulltest-host.json'] = $script:NulltestWirkung
    $e = T-Lauf
    $iRueck = T-Index 'aufgabe rueckweg'
    Pruefe ($iRueck -ge 0) 'ABWEICHUNG: kein Rueckweg'
    Pruefe ($script:T.FlBeiRueckweg -eq 0) "ABWEICHUNG: $($script:T.FlBeiRueckweg) FL-Prozess(e) leben waehrend des Rueckwegs"
    Pruefe ((T-LetzterIndex 'aufgabe pruefen') -gt $iRueck) 'ABWEICHUNG: pruefen nicht nach dem Rueckweg'
    Pruefe (-not (T-Hat 'szenario snapshot-runde01') -and -not (T-Hat 'szenario u40')) 'ABWEICHUNG: Folgeszenarien liefen'
    $roh = [IO.File]::ReadAllText($script:K.Roh)
    Pruefe ([regex]::Matches($roh, 'entfallen: Rueckweg nach ABWEICHUNG').Count -ge 2) 'ABWEICHUNG: Rohdatei ohne Zeilen "entfallen"'
    Pruefe ($e.Code -eq 4 -and $e.Urteil -eq 'VERFEHLT' -and $e.Zusatz -match 'entfallen: snapshot-runde01\.json, u40-aktivitaetsgate\.json' -and $e.Zusatz -match 'rueckweg: ') "ABWEICHUNG: Exit $($e.Code) $($e.Urteil) '$($e.Zusatz)'"
    foreach ($u in @('KETTE', 'VERSATZ')) {
        Testfall $u $script:FuenfSzenarien
        $script:T.NulltestUrteil = $u
        $script:T.NulltestCode = 5
        $script:T.Szenarien['nulltest-host.json'] = $script:NulltestWirkung
        $e = T-Lauf
        Pruefe (-not (T-Hat 'aufgabe rueckweg')) "${u}: Rueckweg gefahren"
        Pruefe ((T-Hat 'szenario snapshot-runde01') -and (T-Hat 'szenario u40')) "${u}: Folgeszenarien liefen nicht"
        Pruefe ($e.Code -eq 3) "${u}: Exit $($e.Code)"
    }
    Testfall 'FL laeuft beim Rueckweg' $script:FuenfSzenarien
    $script:T.NulltestUrteil = 'ABWEICHUNG'
    $script:T.NulltestCode = 4
    $script:T.Szenarien['nulltest-host.json'] = { $script:T.Haengend = T-Prozess 'Nakama-Diagnose.flp - FL Studio 2026' $null $true $false; & $script:NulltestWirkung }
    $e = T-Lauf
    $textFl = $e.Zusatz
    Pruefe (-not (T-Hat 'aufgabe rueckweg')) 'FL laeuft: Rueckweg trotzdem gestartet'
    Pruefe ($textFl -match "Rueckweg verweigert \(FL laeuft\): PID $($script:T.Haengend) 'Nakama-Diagnose") "FL laeuft: Kopfzeile '$textFl'"
    Testfall 'NAK-41' $script:FuenfSzenarien
    $script:T.NulltestUrteil = 'ABWEICHUNG'
    $script:T.NulltestCode = 4
    $script:T.Szenarien['nulltest-host.json'] = $script:NulltestWirkung
    $script:T.Aufgaben.rueckweg = @{ Exit = 1; Zustand = 'Ready'; Log = @('ABBRUCH: Rueckweg verweigert (NAK-41). Mit -Erzwingen ausdruecklich wiederholen.') }
    $e = T-Lauf
    Pruefe ($e.Zusatz -match 'Rueckweg verweigert \(NAK-41\)') "NAK-41: Kopfzeile '$($e.Zusatz)'"
    Pruefe ($textFl -notmatch 'NAK-41' -and $e.Zusatz -notmatch 'FL laeuft\)') 'FL laeuft und NAK-41 ergeben denselben Grund'
}

# ---------------------------------------------------------------- Einstieg
if ($Selbsttest) {
    try { $code = Fahre-Selbsttest $Nur } catch { Write-Host "SELBSTTEST laufzeit.ps1 WERKZEUGFEHLER: $($_.Exception.Message)"; $code = 2 }
    exit $code
}
if (-not $Ticket) { Write-Host 'LAUFZEIT ohne -Ticket: Voraussetzung fehlt'; exit 3 }
$arbeit = Join-Path $env:LOCALAPPDATA 'evenacadia\nakama-laufzeit'
New-Item -ItemType Directory -Force -Path $arbeit | Out-Null
$kopfSha = (& git -C $Repo rev-parse --short=8 HEAD 2>$null)
if (-not $kopfSha) { $kopfSha = 'ohne-git' }
if (-not $Roh) { $Roh = Join-Path $Repo ("docs\beweise\roh\{0}-laufzeit-{1}.md" -f $Ticket, $kopfSha) }
$script:U = Neue-Umgebung
$script:K = Neuer-Kontext @{
    Ticket = $Ticket; Basis = $Basis; Head = $kopfSha; Repo = $Repo; McpRepo = $McpRepo
    Erzwingen = $Erzwingen; OhneInstallation = $OhneInstallation; Neustart = $Neustart; Beenden = $Beenden
    Projekt = $Projekt; Sample = $Sample; Roh = $Roh; Szenarien = $Szenarien; LoopMidi = $LoopMidi
    PingFristSekunden = $PingFristSekunden; InstallFristSekunden = $InstallFristSekunden; RenderFristSekunden = $RenderFristSekunden
    Arbeit = $arbeit; DiagnoseOrdner = (Join-Path $env:LOCALAPPDATA 'evenacadia\nakama\diagnose'); Leise = $false
}
$ergebnis = Fahre-Lauf
exit ([int]$ergebnis.Code)
