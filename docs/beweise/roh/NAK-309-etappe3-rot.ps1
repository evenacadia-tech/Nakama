#Requires -Version 7.2
<#
NAK-309 Etappe 3 - Rotbeweise nach Paragraph 6.1 (docs/beweise/NAK-309.md), Handlauf M-29 und Einzelbeine.

Je Matrixzeile M-18 bis M-56 und M-74 eine Mutation genau an der Zeile, die die Zusage traegt; nur das betroffene
Bein (C++: nur sein Ziel bauen, dann das Binary; Python und PowerShell: der Selbsttest mit --nur beziehungsweise -Nur,
dazu das ganze Bein A35; Nachbarrepo: pytest); Ruecknahme bytegleich mit SHA-256, LastWriteTime auf jetzt, Bein danach
gruen. Ergebnis je Zeile: docs/beweise/roh/NAK-309-rot-M-NN.txt (mehrere Teile einer Zeile in derselben Datei).

Aufruf vom Workspace-Root:
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Trocken        # jede Fundstelle genau einmal?
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe3-rot.ps1                 # alle Rotbeweise
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Nur M-18,M-74
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Handlauf       # M-29
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Einzelbeine

Grenzen (Auftrag Etappe 3): kein FL, keine Installation, keine Aufgabe \Nakama\*, keine Produktions-Pipe. Mutationen,
die einen Pipenamen aendern, werden nie in ein Binary uebersetzt, das eine Pipe oeffnet: M-18 (2), M-19 (2) und M-24
misst eine Quelltextwache, die die Quelle zur Laufzeit liest; M-74 (a) baut nur B8, und B8 endet vor dem ersten
Prozessor; M-23 startet den Client nur mit leerem Namen. Vor jeder Mutation muss die Datei gleich HEAD sein.
#>
param(
    [string[]]$Nur = @(),
    [switch]$Trocken,
    [switch]$Handlauf,
    [switch]$Einzelbeine,
    [string]$McpRepo = 'C:\Users\phili\Projekte\fl-studio-mcp'
)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$RohOrdner = Join-Path $Repo 'docs\beweise\roh'
$Temp = Join-Path ([IO.Path]::GetTempPath()) 'nakama-nak309-e3-rot'
[void](New-Item -ItemType Directory -Force -Path $Temp)
$env:PYTHONDONTWRITEBYTECODE = '1'
$env:PYTHONIOENCODING = 'utf-8'
$Utf8 = New-Object System.Text.UTF8Encoding($false)
$WerkzeugSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $PSCommandPath).Hash
$Pwsh = (Get-Command pwsh).Source
$Py = (Get-Command py).Source
$McpPy = Join-Path $McpRepo '.venv\Scripts\python.exe'
$Bau = Join-Path $Repo 'eq-copilot\build'
$Bin = @{
    A4b   = Join-Path $Bau 'plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe'
    B8    = Join-Path $Bau 'plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe'
    A1    = Join-Path $Bau 'plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe'
    B14   = Join-Path $Bau 'plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe'
    Probe = Join-Path $Bau 'plugin\EqCopPipeProbe_artefacts\Release\EqCopPipeProbe.exe'
}
$Ziel = @{ A4b = 'EqCopPipeClientTest'; B8 = 'EqCopLebenslaufTest'; A1 = 'EqCopNullTest'; B14 = 'EqCopSonde012ProjectReloadTest'; Probe = 'EqCopPipeProbe' }
# Je Ziel die Quellen dieser Etappe, die es uebersetzt - Bezug der Zeitstempelpruefung, wenn kein Rotbeweis eine
# bestimmte Datei mutiert hat (CMakeLists.txt uebersetzt nichts; B8 liest sie zur Laufzeit).
$EtappenQuellen = @{
    EqCopPipeClientTest            = @('src\ProbePipeRegel.h', 'src\PipeClient.cpp', 'src\PipeClient.h', 'tests\PipeClientLifecycleTestMain.cpp')
    EqCopLebenslaufTest            = @('src\ProbePipeRegel.h', 'src\PipeClient.cpp', 'src\PipeClient.h', 'src\PluginProcessor.cpp', 'src\PluginProcessor.h', 'tests\LebenslaufTestMain.cpp')
    EqCopNullTest                  = @('src\PipeClient.cpp', 'src\PipeClient.h', 'src\PluginProcessor.cpp', 'src\PluginProcessor.h')
    EqCopSonde012ProjectReloadTest = @('src\PipeClient.cpp', 'src\PipeClient.h', 'src\PluginProcessor.cpp', 'src\PluginProcessor.h')
    EqCopPipeProbe                 = @('probe\PipeProbeMain.cpp', 'src\ProbePipeRegel.h', 'src\PipeClient.cpp', 'src\PipeClient.h')
}

function Finde-CMake {
    # wie tools/beweise.ps1 (Finde-CMake): cmake liegt nicht immer im PATH.
    $ausPfad = Get-Command cmake -ErrorAction SilentlyContinue
    if ($ausPfad) { return $ausPfad.Source }
    foreach ($stamm in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
        if (-not $stamm) { continue }
        foreach ($ausgabe in @('BuildTools', 'Community', 'Professional', 'Enterprise')) {
            $k = Join-Path $stamm "Microsoft Visual Studio\2022\$ausgabe\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            if (Test-Path -LiteralPath $k) { return $k }
        }
        $k = Join-Path $stamm 'CMake\bin\cmake.exe'
        if (Test-Path -LiteralPath $k) { return $k }
    }
    throw 'cmake nicht gefunden'
}

function Rel([string]$pfad) {
    if ($pfad.StartsWith($Repo, [StringComparison]::OrdinalIgnoreCase)) { return $pfad.Substring($Repo.Length + 1).Replace('\', '/') }
    return $pfad
}

function Sha([string]$pfad) { return (Get-FileHash -Algorithm SHA256 -LiteralPath $pfad).Hash }

function Lies-Ausgabe([string]$pfad) {
    # Geteilt lesen und kurz wiederholen: ein beendeter Prozessbaum kann die Umleitungsdatei noch einen Moment halten.
    if (-not (Test-Path -LiteralPath $pfad)) { return '' }
    for ($versuch = 1; $versuch -le 10; $versuch++) {
        try {
            $s = [IO.File]::Open($pfad, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete)
            try { return (New-Object IO.StreamReader($s, $Utf8)).ReadToEnd() } finally { $s.Dispose() }
        } catch { Start-Sleep -Milliseconds 500 }
    }
    return "(Ausgabe nicht lesbar: $pfad)"
}

function Fahre([string]$exe, [string[]]$argumente, [string]$ordner, [int]$frist = 900, [hashtable]$umgebung = @{}) {
    # Umleitung in Dateien statt Pipes: kein Warten auf Enkelprozesse wie vctip.exe (NAK-300); harte Frist.
    $n = [guid]::NewGuid().ToString('N').Substring(0, 12)
    $aus = Join-Path $Temp "$n.out"
    $feh = Join-Path $Temp "$n.err"
    $text = @($argumente | ForEach-Object { if ($_ -eq '' -or $_ -match '[\s"]') { '"' + ($_ -replace '"', '\"') + '"' } else { $_ } }) -join ' '
    $alt = @{}
    foreach ($k in $umgebung.Keys) { $alt[$k] = [Environment]::GetEnvironmentVariable($k); [Environment]::SetEnvironmentVariable($k, $umgebung[$k]) }
    try {
        $sp = @{ FilePath = $exe; WorkingDirectory = $ordner; NoNewWindow = $true; PassThru = $true; RedirectStandardOutput = $aus; RedirectStandardError = $feh }
        if ($text) { $sp.ArgumentList = $text }
        $p = Start-Process @sp
        $null = $p.Handle
        if ($p.WaitForExit($frist * 1000)) { $p.WaitForExit(); $code = $p.ExitCode }
        else { try { $p.Kill($true) } catch { }; $null = $p.WaitForExit(10000); $code = "Frist $frist s ueberschritten" }
    } finally {
        foreach ($k in $alt.Keys) { [Environment]::SetEnvironmentVariable($k, $alt[$k]) }
    }
    $o = Lies-Ausgabe $aus
    $e = Lies-Ausgabe $feh
    foreach ($f in @($aus, $feh)) { try { if (Test-Path -LiteralPath $f) { [IO.File]::Delete($f) } } catch { } }
    $zeige = if ($exe -eq $Py) { 'py' } elseif ($exe -eq $Pwsh) { 'pwsh' } elseif ($exe -eq $McpPy) { '<MCP-Repo>\.venv\Scripts\python.exe' } else { Rel $exe }
    return [pscustomobject]@{ Aufruf = ($zeige + $(if ($text) { ' ' + $text } else { '' })); Code = $code; Aus = $o; Fehler = $e; Ordner = $ordner }
}

# ---------------------------------------------------------------- Beine
function Lauf-A4b { @{ Exe = $Bin.A4b; Args = @(); Ordner = $Repo; Art = 'cpp' } }
function Lauf-B8 { @{ Exe = $Bin.B8; Args = @(); Ordner = $Repo; Art = 'cpp' } }
function Lauf-Laufzeit([string]$nur) { @{ Exe = $Pwsh; Args = @('-NoProfile', '-File', 'tools/fl/laufzeit.ps1', '-Selbsttest', '-Nur', $nur); Ordner = $Repo; Art = 'a35' } }
function Lauf-Szenario([string]$nur) { @{ Exe = $Py; Args = @('-3.13', 'tools/fl/szenario.py', '--selbsttest', '--nur', $nur); Ordner = $Repo; Art = 'a35' } }
function Lauf-A35 { @{ Exe = $Py; Args = @('-3.13', 'tools/fl/selbsttest.py'); Ordner = $Repo; Art = 'a35' } }
function Lauf-Pytest([string[]]$ziele) { @{ Exe = $McpPy; Args = @('-m', 'pytest', '-v', '--color=no', '-p', 'no:cacheprovider') + $ziele; Ordner = $McpRepo; Art = 'pytest'; Frist = 300 } }

function Werte([string]$art, [string]$text) {
    $ok = New-Object System.Collections.Generic.List[string]
    $rot = New-Object System.Collections.Generic.List[string]
    $summe = New-Object System.Collections.Generic.List[string]
    # Farbcodes (ANSI) entfernen: pytest faerbt auch in eine umgeleitete Datei, wenn es die Konsole erbt.
    $text = [regex]::Replace($text, '\x1b\[[0-9;]*[A-Za-z]', '')
    $zeilen = @($text -split "`r?`n" | Where-Object { $_.Trim() })
    foreach ($z in $zeilen) {
        $z = $z.TrimEnd()
        switch ($art) {
            'cpp' {
                # Summenzeilen ("<BEIN>-TEST OK - n ..." / "... FEHLGESCHLAGEN - n ...") zaehlen nicht als Pruefung; die
                # Abbruchzeile von B8 vor dem ersten Prozessor (M-74) schon.
                if ($z -match '^\s{2}ok\s') { $ok.Add($z) }
                elseif ($z -match '^[A-Z-]+-TEST (OK|FEHLGESCHLAGEN) - \d') { $summe.Add($z) }
                elseif ($z -match '^\s{2}FEHLER\s' -or $z -match 'FEHLGESCHLAGEN') { $rot.Add($z) }
            }
            'a35' {
                if ($z -match '^\[ok\] ') { $ok.Add($z) }
                elseif ($z -match '^\[ROT\] ') { $rot.Add($z) }
                elseif ($z -match '^SELBSTTEST ') { $summe.Add($z) }
            }
            'pytest' {
                if ($z -match '::\S+ PASSED') { $ok.Add(($z -replace '\s+\[\s*\d+%\]$', '')) }
                elseif ($z -match '::\S+ (FAILED|ERROR)') { $rot.Add(($z -replace '\s+\[\s*\d+%\]$', '')) }
                elseif ($z -match '^(FAILED|ERROR) ' -or $z -match '^=+ .*\b(passed|failed|error)') { $summe.Add($z) }
            }
        }
    }
    if ($art -eq 'cpp' -and $summe.Count -eq 0 -and $zeilen.Count -gt 0 -and -not $rot.Contains($zeilen[-1].TrimEnd())) { $summe.Add($zeilen[-1].TrimEnd()) }
    return [pscustomobject]@{ Ok = $ok; Rot = $rot; Summe = $summe }
}

function Fahre-Lauf($lauf) {
    $r = Fahre $lauf.Exe $lauf.Args $lauf.Ordner $(if ($lauf.Frist) { $lauf.Frist } else { 900 })
    $w = Werte $lauf.Art ($r.Aus + "`n" + $r.Fehler)
    return [pscustomobject]@{ Aufruf = $r.Aufruf; Code = $r.Code; Ok = $w.Ok; Rot = $w.Rot; Summe = $w.Summe; Roh = $r }
}

function Baue([string[]]$ziele, [string]$quelle, $zeilen) {
    # Nur das Ziel des Beins (cmake --build --target) und danach der Zeitstempel des Binaries gegen die Quelle (NAK-230).
    $gut = $true
    foreach ($z in $ziele) {
        $r = Fahre $script:Cmake @('--build', 'eq-copilot/build', '--config', 'Release', '--target', $z) $Repo 2400
        $zeilen.Add("Aufruf: cmake --build eq-copilot/build --config Release --target $z")
        $zeilen.Add("Exitcode: $($r.Code)")
        foreach ($f in @(($r.Aus + "`n" + $r.Fehler) -split "`r?`n" | Where-Object { $_ -match '\berror\b|: Fehler|fatal' } | Select-Object -First 12)) { $zeilen.Add("  " + $f.Trim()) }
        $key = @($Ziel.Keys | Where-Object { $Ziel[$_] -eq $z })[0]
        if ($key) {
            $bezug = if ($quelle) { @($quelle) } else { @($EtappenQuellen[$z] | ForEach-Object { Join-Path $Repo "eq-copilot\plugin\$_" }) }
            $binZeit = (Get-Item -LiteralPath $Bin[$key]).LastWriteTime
            foreach ($q in $bezug) {
                $qZeit = (Get-Item -LiteralPath $q).LastWriteTime
                $juenger = $binZeit -gt $qZeit
                $zeilen.Add(("Binary {0}: {1:yyyy-MM-dd HH:mm:ss.fff}, Quelle {2}: {3:yyyy-MM-dd HH:mm:ss.fff} - Binary juenger: {4}" -f (Rel $Bin[$key]), $binZeit, (Rel $q), $qZeit, $(if ($juenger) { 'ja' } else { 'NEIN' })))
                if (-not $juenger) { $gut = $false }
            }
        }
        if ($r.Code -ne 0) { $gut = $false }
    }
    return $gut
}

# ---------------------------------------------------------------- Mutation
function Lies-Datei([string]$pfad) {
    $bytes = [IO.File]::ReadAllBytes($pfad)
    $bom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
    $start = if ($bom) { 3 } else { 0 }
    $text = $Utf8.GetString($bytes, $start, $bytes.Length - $start)
    $crlf = ([regex]::Matches($text, "`r`n")).Count
    $lf = ([regex]::Matches($text, "(?<!`r)`n")).Count
    if ($crlf -gt 0 -and $lf -gt 0) { throw "$pfad hat gemischte Zeilenenden ($crlf CRLF, $lf LF)" }
    # -split mit 0: alle Teile, auch das leere hinter dem letzten Zeilenende (ein negativer Wert zaehlte von rechts).
    return @{ Bytes = $bytes; Bom = $bom; Text = $text; Eol = $(if ($crlf -gt 0) { "`r`n" } else { "`n" }); Zeilen = [string[]]($text -split "`r?`n", 0) }
}

function Block([string]$hier) { return , [string[]]($hier -split "`r?`n") }

function Finde-Stellen($d, $ersetzungen) {
    # Jede Fundstelle muss als ganze Zeilen genau einmal vorkommen.
    $stellen = @()
    foreach ($e in $ersetzungen) {
        $alt = [string[]](Block $e.Alt)
        $treffer = @()
        for ($i = 0; $i -le $d.Zeilen.Count - $alt.Count; $i++) {
            $gleich = $true
            for ($j = 0; $j -lt $alt.Count; $j++) { if ($d.Zeilen[$i + $j] -cne $alt[$j]) { $gleich = $false; break } }
            if ($gleich) { $treffer += $i }
        }
        if ($treffer.Count -ne 1) { throw ("Fundstelle {0}-mal statt genau einmal: {1}" -f $treffer.Count, $alt[0].Trim()) }
        $stellen += [pscustomobject]@{ Index = $treffer[0]; Alt = $alt; Neu = [string[]](Block $e.Neu) }
    }
    return @($stellen | Sort-Object Index)
}

function Mutierte-Bytes($d, $stellen) {
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.AddRange($d.Zeilen)
    foreach ($s in @($stellen | Sort-Object Index -Descending)) {
        $zeilen.RemoveRange($s.Index, $s.Alt.Count)
        $zeilen.InsertRange($s.Index, [string[]]$s.Neu)
    }
    $text = [string]::Join($d.Eol, $zeilen)
    $inhalt = $Utf8.GetBytes($text)
    if ($d.Bom) { return [byte[]](@(0xEF, 0xBB, 0xBF) + $inhalt) }
    return [byte[]]$inhalt
}

function Stellen-Text($stellen) {
    return (@($stellen | ForEach-Object { if ($_.Alt.Count -eq 1) { "$($_.Index + 1)" } else { "$($_.Index + 1)-$($_.Index + $_.Alt.Count)" } }) -join ', :')
}

function Git-Sauber([string]$repo, [string]$datei) {
    & git -C $repo diff --quiet -- $datei
    return ($LASTEXITCODE -eq 0)
}

# ---------------------------------------------------------------- Basislauf im Nachbarrepo (heute rot, ohne Umbau)
function Basislauf-Mcp([string]$test, $zeilen) {
    # Der neue Test gegen den unveraenderten Basisstand 01f72386 des Nachbarrepos: Baum per git archive in einen
    # Temp-Ordner, dazu die Testdatei des Etappenstands; das Repo selbst bleibt unberuehrt.
    $basis = '01f72386bfc7eab507a40b9c96ef9e2e2a0265cc'
    $ordner = Join-Path $Temp 'mcp-basis-01f72386'
    if (-not (Test-Path -LiteralPath (Join-Path $ordner 'src'))) {
        $zip = Join-Path $Temp 'mcp-basis-01f72386.zip'
        & git -C $McpRepo archive --format=zip -o $zip $basis
        if ($LASTEXITCODE -ne 0) { throw 'git archive des Basisstands scheitert' }
        Expand-Archive -LiteralPath $zip -DestinationPath $ordner -Force
    }
    Copy-Item -LiteralPath (Join-Path $McpRepo 'tests\test_request_id.py') -Destination (Join-Path $ordner 'tests\test_request_id.py') -Force
    $r = Fahre $McpPy @('-m', 'pytest', '-v', '--color=no', '-p', 'no:cacheprovider', "tests/test_request_id.py::$test") $ordner 300 @{ PYTHONPATH = (Join-Path $ordner 'src') }
    $w = Werte 'pytest' ($r.Aus + "`n" + $r.Fehler)
    $zeilen.Add('')
    $zeilen.Add("--- LAUF DES NEUEN TESTS GEGEN DEN BASISSTAND (fl-studio-mcp $($basis.Substring(0, 8)), git archive, PYTHONPATH=<Basis>\src) ---")
    $zeilen.Add("Aufruf: $($r.Aufruf)")
    $zeilen.Add("Exitcode: $($r.Code)")
    foreach ($z in @($w.Rot) + @($w.Ok) + @($w.Summe)) { $zeilen.Add("  $z") }
    foreach ($z in @(($r.Aus -split "`r?`n") | Where-Object { $_ -match '^E\s' } | Select-Object -First 6)) { $zeilen.Add("  $($z.TrimEnd())") }
}

# ---------------------------------------------------------------- ein Rotbeweis
function Rotbeweis($s) {
    $repoPfad = if ($s.Repo -eq 'mcp') { $McpRepo } else { $Repo }
    $datei = Join-Path $repoPfad $s.Datei
    $d = Lies-Datei $datei
    $stellen = Finde-Stellen $d $s.Ersetzungen
    if ($Trocken) { Write-Host ("[trocken] {0} {1}: {2}:{3} ok" -f $s.Zeile, $s.Teil, $s.Datei, (Stellen-Text $stellen)); return }
    if (-not (Git-Sauber $repoPfad $s.Datei)) { throw "$($s.Datei) weicht von HEAD ab - kein Rotbeweis auf ungeprueftem Stand" }
    $kopf = (& git -C $repoPfad rev-parse --short=8 HEAD).Trim()
    $shaVorher = Sha $datei
    $neu = Mutierte-Bytes $d $stellen
    $z = New-Object System.Collections.Generic.List[string]
    $z.Add("NAK-309 Etappe 3 - Rotbeweis $($s.Zeile) $($s.Teil)")
    $z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}{2}" -f (Get-Date), $kopf, $(if ($s.Repo -eq 'mcp') { ' (fl-studio-mcp, evenacadia-local)' } else { '' })))
    $z.Add("Zusage: $($s.Zusage)")
    $z.Add("Bein:   $($s.Bein)")
    $z.Add("Datei:  $($s.Datei):$(Stellen-Text $stellen)")
    $z.Add("SHA-256 vorher: $shaVorher (Datei gleich HEAD: ja)")
    $z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe3-rot.ps1 (SHA-256 $WerkzeugSha)")
    if ($s.Basislauf) { Basislauf-Mcp $s.Basislauf $z }
    elseif ($s.OhneBasislauf) { $z.Add(''); $z.Add("Lauf gegen den Basisstand: $($s.OhneBasislauf)") }
    $z.Add('')
    $z.Add('--- MUTIERTE STELLE (die Zeile, die die Zusage traegt) ---')
    foreach ($st in $stellen) {
        $z.Add("vorher (Zeile $($st.Index + 1)):")
        foreach ($l in $st.Alt) { $z.Add($l) }
        $z.Add('nachher:')
        foreach ($l in $st.Neu) { $z.Add($l) }
    }

    $laeufe = @()
    $bauGut = $true
    try {
        [IO.File]::WriteAllBytes($datei, $neu)
        (Get-Item -LiteralPath $datei).LastWriteTime = Get-Date
        $z.Add('')
        $z.Add('--- BAU UNTER MUTATION ---')
        if ($s.Bau) { $bauGut = Baue $s.Bau $datei $z }
        elseif ($s.BauGrund) { $z.Add("kein Bau: $($s.BauGrund)") }
        else { $z.Add('kein Bau (Python, PowerShell oder Szenariodatei: das Bein liest die Quelle beim Lauf)') }
        if ($bauGut) { foreach ($l in $s.Laeufe) { $laeufe += Fahre-Lauf $l } }
    } finally {
        [IO.File]::WriteAllBytes($datei, $d.Bytes)
        (Get-Item -LiteralPath $datei).LastWriteTime = Get-Date
    }
    $shaNachher = Sha $datei
    $sauber = Git-Sauber $repoPfad $s.Datei

    $z.Add('')
    $z.Add('--- LAUF UNTER MUTATION ---')
    if (-not $bauGut) { $z.Add('Bau unter Mutation gescheitert - kein Lauf, kein Beleg.') }
    foreach ($l in $laeufe) {
        $z.Add("Aufruf: $($l.Aufruf)")
        $z.Add("Exitcode: $($l.Code)")
        foreach ($r in $l.Rot) { $z.Add("  $r") }
        foreach ($r in $l.Summe) { $z.Add("  $r") }
        $z.Add('')
    }
    $alleRot = New-Object System.Collections.Generic.List[string]
    foreach ($l in $laeufe) { foreach ($r in $l.Rot) { if (-not $alleRot.Contains($r)) { $alleRot.Add($r) } } }
    $z.Add('Gefallene Pruefungen:')
    if ($alleRot.Count -eq 0) { $z.Add('  keine') }
    foreach ($r in $alleRot) { $z.Add("  $r") }
    $z.Add('')
    $treffer = @($s.Erwartet | ForEach-Object { $m = $_.Muster; [pscustomobject]@{ Text = $_.Text; Gefallen = (@($alleRot | Where-Object { $_ -match $m }).Count -gt 0) } })
    $ja = $bauGut -and $alleRot.Count -gt 0 -and @($treffer | Where-Object { -not $_.Gefallen }).Count -eq 0
    $z.Add("Die Zusage faellt an der erwarteten Pruefung: $(if ($ja) { 'JA' } else { 'NEIN' })")
    foreach ($t in $treffer) { $z.Add("  erwartet: $($t.Text)  -> $(if ($t.Gefallen) { 'gefallen' } else { 'NICHT gefallen' })") }
    $z.Add('')
    $mit = @($alleRot | Where-Object { $r = $_; @($s.Erwartet | Where-Object { $r -match $_.Muster }).Count -eq 0 })
    if ($mit.Count -eq 0) { $z.Add('Mitgefallen: keine') }
    else {
        $z.Add('Mitgefallen:')
        foreach ($r in $mit) {
            $grund = 'nicht vorhergesagt'
            if ($s.Mit) { foreach ($k in $s.Mit.Keys) { if ($r -match $k) { $grund = $s.Mit[$k]; break } } }
            $z.Add("  $r")
            $z.Add("      Grund: $grund")
        }
    }
    $z.Add('')
    $gruen = if ($laeufe.Count -gt 0) { @($laeufe[-1].Ok) } else { @() }
    $z.Add("Unter derselben Mutation weiter gruen (Beleg gegen den Nebeneffekt): $($gruen.Count)")
    foreach ($g in $gruen) { $z.Add("  $g") }
    if ($s.Hinweis) { $z.Add(''); $z.Add("Hinweis: $($s.Hinweis)") }

    $z.Add('')
    $z.Add('--- LAUF NACH RUECKNAHME ---')
    $z.Add("Quelle bytegleich zurueckgespielt: $(if ($shaNachher -eq $shaVorher) { 'ja' } else { 'NEIN' }) (SHA-256 $shaNachher); LastWriteTime auf jetzt gesetzt; git diff leer: $(if ($sauber) { 'ja' } else { 'NEIN' })")
    $nachGut = $shaNachher -eq $shaVorher -and $sauber
    if ($s.Bau) { if (-not (Baue $s.Bau $datei $z)) { $nachGut = $false } }
    elseif ($s.NachBauGrund) { $z.Add("kein Bau: $($s.NachBauGrund)") }
    foreach ($l in $s.Laeufe) {
        $n = Fahre-Lauf $l
        $z.Add("Aufruf: $($n.Aufruf)")
        $z.Add("Exitcode: $($n.Code)")
        foreach ($r in $n.Rot) { $z.Add("  $r") }
        foreach ($r in $n.Summe) { $z.Add("  $r") }
        if ($n.Code -ne 0) { $nachGut = $false }
    }
    $z.Add('')
    $z.Add("Bein nach Ruecknahme gruen: $(if ($nachGut) { 'ja' } else { 'NEIN' })")

    $roh = Join-Path $RohOrdner ("NAK-309-rot-{0}.txt" -f $s.Zeile)
    $text = [string]::Join("`n", $z) + "`n"
    if ($script:Geschrieben.Contains($roh)) { $text = "`n==============================================================================`n" + $text; [IO.File]::AppendAllText($roh, $text, $Utf8) }
    else { [IO.File]::WriteAllText($roh, $text, $Utf8); [void]$script:Geschrieben.Add($roh) }
    Write-Host ("{0} {1}: faellt an der Zusagezeile {2}, nach Ruecknahme gruen {3}, mitgefallen {4}" -f $s.Zeile, $s.Teil, $(if ($ja) { 'JA' } else { 'NEIN' }), $(if ($nachGut) { 'ja' } else { 'NEIN' }), $mit.Count)
}

# ---------------------------------------------------------------- Mutationen
$S = New-Object System.Collections.Generic.List[object]
function Spec([hashtable]$h) { $S.Add([pscustomobject]$h) }
function E([string]$muster, [string]$text) { [pscustomobject]@{ Muster = $muster; Text = $text } }

$A4bOhneBau = 'A4b uebersetzt probe/PipeProbeMain.cpp nicht; die Quelltextwache liest die Quelle zur Laufzeit (__FILE__). EqCopPipeProbe wird unter dieser Mutation nicht gebaut: ein Probe-Binary, das einen anderen als den geprueften Namen oeffnen koennte, soll nie entstehen (Auftrag: keine Produktions-Pipe).'
$A4bHeuteRot = 'ohne Umbau nicht moeglich - der neue Fall steht im Testbinary des Etappenstands; das Binary des Basis-SHA kennt ihn nicht (Gegenprobe steht in der Matrixzeile).'
$A35HeuteRot = 'ohne Umbau nicht moeglich - Selbsttest und Code stehen in derselben Datei (Gegenprobe steht in der Matrixzeile).'
$A4bBein = 'A4b EqCopPipeClientTest (eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp)'
$A35Bein = 'A35 (tools/fl/selbsttest.py); gezielt der Fall ueber -Nur beziehungsweise --nur'

# --- M-18 bis M-28: PipeProbe (A4b) ---
Spec @{ Zeile = 'M-18'; Teil = '(1) probePipeName ohne Argument'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Kein Argument -> Probe-Name: gewaehlt wird \\.\pipe\evenacadia.eq-copilot.m2probe, nie kPipeName.'
    Ersetzungen = @(@{
Alt = @'
    return argc > 1 ? juce::String (argv[1]) : juce::String (kV2ProbePipeName);
'@
Neu = @'
    return argc > 1 ? juce::String (argv[1]) : juce::String (juce::CharPointer_UTF16 (kPipeName)); // MUTATION: ohne Argument kPipeName (Stand am Basis-SHA)
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_pipe_ohne_argument_waehlt_probe_namen\s+\[' 'probe_pipe_ohne_argument_waehlt_probe_namen')) }

Spec @{ Zeile = 'M-18'; Teil = '(2) Namenswahl in main ohne probePipeName'; Datei = 'eq-copilot/plugin/probe/PipeProbeMain.cpp'; Bein = $A4bBein
    Zusage = 'Namensentscheidung ueber probePipeName vor serverErwartungFuer und vor baueClient (Quelltextwache).'
    Ersetzungen = @(@{
Alt = @'
    const juce::String pipeName = eqcop::probe::probePipeName (argc, argv);
'@
Neu = @'
    const juce::String pipeName = argc > 1 ? juce::String (argv[1]) : juce::String (juce::CharPointer_UTF16 (eqcop::kPipeName)); // MUTATION: Namenswahl ohne probePipeName (Stand am Basis-SHA)
'@ })
    BauGrund = $A4bOhneBau; NachBauGrund = 'wie unter Mutation (Quelltextwache)'; Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_main_prueft_pipenamen_vor_dem_client' 'probe_main_prueft_pipenamen_vor_dem_client')) }

Spec @{ Zeile = 'M-19'; Teil = '(1) Produktionsname gross- und kleinschreibungsgenau verglichen'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Produktionsname -> Ablehnung vor jedem Pipezugriff: PROBE ABGELEHNT (Pipename produktion), Exit 64, auch in anderer Gross- und Kleinschreibung.'
    Ersetzungen = @(@{
Alt = @'
    if (name.equalsIgnoreCase (juce::String (juce::CharPointer_UTF16 (kPipeName))))
'@
Neu = @'
    if (name == juce::String (juce::CharPointer_UTF16 (kPipeName))) // MUTATION: Vergleich gross- und kleinschreibungsgenau
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/produktion_grossgeschrieben' 'produktion_grossgeschrieben'), (E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/produktion_gemischt' 'produktion_gemischt'))
    Hinweis = 'Unter der Mutation lehnt die Regel beide Schreibweisen weiter ab, aber als fremd statt als produktion: die Praefixpruefung ist gross- und kleinschreibungsgenau, und die Erlaubnisliste kennt den Namen nicht. Der Weg "Grossschreibung zugelassen" aus der Matrixzeile tritt deshalb nicht ein; die Zusage (Urteil produktion) faellt an genau den beiden Faellen.' }

Spec @{ Zeile = 'M-19'; Teil = '(2) Ruecksprung mit Exit 64 entfernt'; Datei = 'eq-copilot/plugin/probe/PipeProbeMain.cpp'; Bein = $A4bBein
    Zusage = 'Abgelehnter Name -> Ruecksprung mit Exit 64 vor der Servererwartung und vor dem ersten baueClient (Quelltextwache).'
    Ersetzungen = @(@{
Alt = @'
        return kExitPipename;
'@
Neu = @'
        // MUTATION: Ruecksprung mit Exit 64 entfernt
'@ })
    BauGrund = $A4bOhneBau; NachBauGrund = 'wie unter Mutation (Quelltextwache)'; Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_main_prueft_pipenamen_vor_dem_client' 'probe_main_prueft_pipenamen_vor_dem_client')) }

Spec @{ Zeile = 'M-20'; Teil = 'nur istProbePipename als Regel'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Probe-Name -> Verbindung: v2 genau, v2 mit Suffix (darunter ....m2probe.nak123r1 aus NAK-123) und v3-Probenamen sind zugelassen; istProbePipename bleibt unveraendert.'
    Ersetzungen = @(@{
Alt = @'
    const juce::String v2 (kV2ProbePipeName);
    if (name == v2)
        return PipeUrteil::zugelassen;
    if (name.startsWith (v2 + "."))
    {
        const auto suffix = name.substring (v2.length() + 1);
        if (suffix.isNotEmpty()
            && suffix.containsOnly ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-"))
            return PipeUrteil::zugelassen;
    }
'@
Neu = @'
    // MUTATION: v2-Zweig entfernt - nur istProbePipename entscheidet
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b))
    OhneBasislauf = 'entfaellt - Regressionswache (am Basis-SHA wird jeder Name angenommen, also auch diese).'
    Erwartet = @((E 'FEHLER\s+probe_pipe_laesst_probe_namen_zu/v2_suffix_nak123r1' 'v2_suffix_nak123r1 (Name der NAK-123-Laeufe)'), (E 'FEHLER\s+probe_pipe_laesst_probe_namen_zu/v2_genau' 'v2_genau'))
    Mit = [ordered]@{ 'probe_pipe_laesst_probe_namen_zu/v2_' = 'dieselbe Regel: jeder v2-Probename faellt ohne den v2-Zweig'; 'probe_pipe_ohne_argument_waehlt_probe_namen' = 'der Vorgabename ohne Argument ist der v2-Probename; sein Urteil faellt mit dem v2-Zweig' } }

Spec @{ Zeile = 'M-21'; Teil = 'leeres Argument wie ohne Argument'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Leeres Argument -> Ablehnung leer, Exit 64 - nie der Vorgabename, nie der Rueckfall in PipeClient.'
    Ersetzungen = @(@{
Alt = @'
    return argc > 1 ? juce::String (argv[1]) : juce::String (kV2ProbePipeName);
'@
Neu = @'
    return argc > 1 && argv[1][0] != '\0' ? juce::String (argv[1]) : juce::String (kV2ProbePipeName); // MUTATION: leeres Argument wie ohne Argument
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_pipe_ohne_argument_waehlt_probe_namen/mit_argument_gilt_das_argument' 'probe_pipe_ohne_argument_waehlt_probe_namen/mit_argument_gilt_das_argument (leer bleibt leer)')) }

Spec @{ Zeile = 'M-22'; Teil = '(1) Sperrliste statt Erlaubnisliste'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Fremder Name (v3-Produktionsform, Traversal, beliebig) -> Ablehnung fremd (Erlaubnisliste statt Sperrliste), Exit 64.'
    Ersetzungen = @(@{
Alt = @'
    if (! name.startsWith (praefix) || name.substring (praefix.length()).containsAnyOf ("\\/"))
        return PipeUrteil::fremd;
'@
Neu = @'
    if (! name.startsWith (praefix) || name.substring (praefix.length()).containsAnyOf ("\\/"))
        return PipeUrteil::zugelassen; // MUTATION: Sperrliste - abgelehnt werden nur leer und kPipeName
'@ }, @{
Alt = @'
    return nakama::ipc::istProbePipename (name.toStdString()) ? PipeUrteil::zugelassen
                                                              : PipeUrteil::fremd;
'@
Neu = @'
    return nakama::ipc::istProbePipename (name.toStdString()) ? PipeUrteil::zugelassen
                                                              : PipeUrteil::zugelassen; // MUTATION: Sperrliste
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/m22_v3_produktionsform' 'm22_v3_produktionsform'), (E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/m22_traversal_v2' 'm22_traversal_v2'), (E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/m22_beliebig' 'm22_beliebig'))
    Mit = [ordered]@{ 'probe_pipe_lehnt_produktionsnamen_ab/m22_' = 'dieselbe Erlaubnisliste: jeder fremde Name wird unter der Sperrliste zugelassen' } }

Spec @{ Zeile = 'M-22'; Teil = '(2) Trennerregel entfernt (Abweichung vom Bauplan)'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Ein Trenner (\ oder /) hinter \\.\pipe\ macht jeden Namen fremd - sonst oeffnete \\.\pipe\<probe>\..\<produktion> die Produktion ueber den v3-Zweig.'
    Ersetzungen = @(@{
Alt = @'
    if (! name.startsWith (praefix) || name.substring (praefix.length()).containsAnyOf ("\\/"))
'@
Neu = @'
    if (! name.startsWith (praefix)) // MUTATION: Trennerregel entfernt
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+probe_pipe_lehnt_produktionsnamen_ab/m22_traversal_v3' 'm22_traversal_v3 (\\.\pipe\evenacadia.nakama.v3.probe.x\..\evenacadia.eq-copilot.v1)'))
    Hinweis = 'm22_traversal_v2 und m22_traversal_schraegstrich bleiben unter dieser Mutation gruen: der v2-Zweig verlangt den Namen genau oder mit Punkt-Suffix, und istProbePipename sieht den v3-Praefix nicht. Nur der v3-Zweig laesst das Traversal ohne Trennerregel durch - istProbePipename prueft nur den Praefix. Deshalb traegt die Trennerregel die Zusage.' }

Spec @{ Zeile = 'M-23'; Teil = '(1) Rueckfall auf kPipeName'; Datei = 'eq-copilot/plugin/src/PipeClient.cpp'; Bein = $A4bBein
    Zusage = 'Kein Rueckfall auf die Produktions-Pipe: ein PipeClient mit leerem Namen traegt den leeren Namen.'
    Ersetzungen = @(@{
Alt = @'
      pipeName (name),
'@
Neu = @'
      pipeName (name.isNotEmpty() ? name : juce::String (juce::CharPointer_UTF16 (kPipeName))), // MUTATION: Rueckfall auf kPipeName (Stand am Basis-SHA)
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+pipeclient_ohne_namen_verbindet_nicht/name_bleibt_leer' 'pipeclient_ohne_namen_verbindet_nicht/name_bleibt_leer'))
    Hinweis = 'Der Fall startet den Client nur, wenn sein Name leer ist; unter der Mutation traegt er kPipeName und kehrt vor start() zurueck - kein Pipezugriff.' }

Spec @{ Zeile = 'M-23'; Teil = '(2) Leerzweig in start() entfernt'; Datei = 'eq-copilot/plugin/src/PipeClient.cpp'; Bein = $A4bBein
    Zusage = 'start() mit leerem Namen versucht keine Verbindung: verbindungsVersuche bleibt 0, Status getrennt, letzterFehler "kein Pipename".'
    Ersetzungen = @(@{
Alt = @'
    if (pipeName.isEmpty())
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "kein Pipename";
        return;
    }
'@
Neu = @'
    // MUTATION: Leerzweig in start() entfernt
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+pipeclient_ohne_namen_verbindet_nicht/start_versucht_nichts' 'pipeclient_ohne_namen_verbindet_nicht/start_versucht_nichts'))
    Mit = [ordered]@{ 'pipeclient_ohne_namen_verbindet_nicht/nach_stop_kein_versuch' = 'dieselbe Zusage: ohne den Leerzweig versucht start() eine Verbindung auf den leeren Namen, nach stop() steht der Versuchszaehler bei 1' }
    Hinweis ='Unter der Mutation startet der Client mit leerem Namen; CreateFileW mit einem leeren Namen oeffnet keine Pipe (PipeClient.cpp: CreateFileW (pipeName.toWideCharPointer(), ...)).' }

Spec @{ Zeile = 'M-24'; Teil = '{} am v2-Client'; Datei = 'eq-copilot/plugin/src/PluginProcessor.cpp'; Bein = $A4bBein
    Zusage = 'Kein Produktverhalten geaendert: das Produkt nennt kPipeName ausdruecklich (v2PipeNameDesBaus, produktVerdrahtung, verdrahtung.v2PipeName statt {}).'
    Ersetzungen = @(@{
Alt = @'
            verdrahtung.v2PipeName,
'@
Neu = @'
            {}, // MUTATION: leerer Name wie am Basis-SHA
'@ })
    BauGrund = 'A4b uebersetzt den Prozessor nicht; die Quelltextwache liest src/PluginProcessor.cpp zur Laufzeit (__FILE__). Kein Prozessorziel wird unter der Mutation gebaut.'
    NachBauGrund = 'wie unter Mutation (Quelltextwache)'; Laeufe = @((Lauf-A4b))
    OhneBasislauf = 'entfaellt - heute nicht messbar (bis zum Bau trug der Rueckfall die Verbindung).'
    Erwartet = @((E 'FEHLER\s+produkt_nennt_v2_pipe_ausdruecklich' 'produkt_nennt_v2_pipe_ausdruecklich')) }

Spec @{ Zeile = 'M-25'; Teil = '(1) Praedikat verlangt zehn neue ACKs'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'A bis zur geprueften Konfliktantwort verbunden -> Konfliktende, wenn Status verbunden, v2, Versuche gleich, bestaetigte ACKs groesser und konflikt falsch.'
    Ersetzungen = @(@{
Alt = @'
        && jetzt.heartbeatsBestaetigt > vorher.heartbeatsBestaetigt
'@
Neu = @'
        && jetzt.heartbeatsBestaetigt > vorher.heartbeatsBestaetigt + 10 // MUTATION: verlangt zehn neue ACKs
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b))
    OhneBasislauf = 'entfaellt - Regressionswache (am Basis-SHA endet dieser Fall ebenso).'
    Erwartet = @((E 'FEHLER\s+konfliktende_mit_neuem_ack_bestaetigt\s+\[' 'konfliktende_mit_neuem_ack_bestaetigt'))
    Mit = [ordered]@{ 'konfliktende_wartet_auf_ack_ohne_konflikt/folgendes_ack_ohne_konflikt' = 'dieselbe Bedingung: das folgende ACK ohne Konflikt (M-28) liegt nur zwei ACKs hinter dem Stand und ist ebenfalls ein Konfliktende' } }

Spec @{ Zeile = 'M-25'; Teil = '(2) main wartet wieder auf das blosse Fallen des Flags'; Datei = 'eq-copilot/plugin/probe/PipeProbeMain.cpp'; Bein = $A4bBein
    Zusage = 'main zieht den Stand direkt nach b->stop() und wartet auf konfliktEndeBestaetigt statt auf das Fallen des Flags (Quelltextwache).'
    Ersetzungen = @(@{
Alt = @'
    if (! warteAuf (100, [&] { return eqcop::probe::konfliktEndeBestaetigt (vorher, a->snapshot()); }))
'@
Neu = @'
    if (! warteAuf (100, [&] { return ! a->snapshot().konflikt; })) // MUTATION: altes Flagpraedikat (Stand am Basis-SHA)
'@ })
    BauGrund = $A4bOhneBau; NachBauGrund = 'wie unter Mutation (Quelltextwache)'; Laeufe = @((Lauf-A4b))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E 'FEHLER\s+probe_main_wartet_auf_konfliktende_praedikat' 'probe_main_wartet_auf_konfliktende_praedikat')) }

Spec @{ Zeile = 'M-26'; Teil = 'Statusbedingung entfernt'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'A mit Verbindungsabbruch -> kein Konfliktende; nach der Frist PROBE FEHLGESCHLAGEN (Konflikt-Ende ohne bestaetigtes ACK), Exit 1.'
    Ersetzungen = @(@{
Alt = @'
    return jetzt.status == PipeClient::Status::verbunden
'@
Neu = @'
    return true // MUTATION: Statusbedingung entfernt
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+konfliktende_nicht_bei_abbruch/nur_status' 'konfliktende_nicht_bei_abbruch/nur_status'))
    Hinweis = 'konfliktende_nicht_bei_abbruch/abbau bleibt gruen: der Abbau setzt auch Protokoll 0, das haelt die zweite Bedingung. Der Stand nur_status (getrennt, Protokoll 2) trennt die Statusbedingung davon.' }

Spec @{ Zeile = 'M-27'; Teil = 'Versuchsbedingung entfernt'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Neuaufbau -> kein Konfliktende, auch wenn der neu gezaehlte ACK-Zaehler den alten Wert ueberholt.'
    Ersetzungen = @(@{
Alt = @'
        && jetzt.verbindungsVersuche == vorher.verbindungsVersuche
'@
Neu = @'
        // MUTATION: Versuchsbedingung entfernt
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b)); OhneBasislauf = $A4bHeuteRot
    Erwartet = @((E 'FEHLER\s+konfliktende_nicht_nach_neuaufbau' 'konfliktende_nicht_nach_neuaufbau')) }

Spec @{ Zeile = 'M-28'; Teil = 'Konfliktbedingung entfernt'; Datei = 'eq-copilot/plugin/src/ProbePipeRegel.h'; Bein = $A4bBein
    Zusage = 'Erst das ACK ohne Konflikt beendet; das erste neue ACK mit konflikt zaehlt nicht.'
    Ersetzungen = @(@{
Alt = @'
        && ! jetzt.konflikt;
'@
Neu = @'
        ; // MUTATION: Konfliktbedingung entfernt
'@ })
    Bau = @('EqCopPipeClientTest'); Laeufe = @((Lauf-A4b))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E 'FEHLER\s+konfliktende_wartet_auf_ack_ohne_konflikt/erstes_ack_mit_konflikt' 'konfliktende_wartet_auf_ack_ohne_konflikt/erstes_ack_mit_konflikt')) }

# --- M-74: Testbau oeffnet die Produktions-Pipe nie (B8) ---
Spec @{ Zeile = 'M-74'; Teil = '(a) Testzweig von v2PipeNameDesBaus nennt kPipeName'; Datei = 'eq-copilot/plugin/src/PluginProcessor.cpp'
    Bein = 'B8 EqCopLebenslaufTest (eq-copilot/plugin/tests/LebenslaufTestMain.cpp, Teil 0)'
    Zusage = 'Ein Test, der den Prozessor baut, oeffnet die Produktions-Pipe nie: im Testbau traegt der v2-Client einen Probe-Namen; das Bein misst die Namenswahl vor dem ersten Prozessor und endet bei einem Verstoss vor jeder Konstruktion.'
    Ersetzungen = @(@{
Alt = @'
    return juce::String ("\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.testbau");
'@
Neu = @'
    return juce::String (juce::CharPointer_UTF16 (kPipeName)); // MUTATION: der Testbau nennt die Produktions-Pipe
'@ })
    Bau = @('EqCopLebenslaufTest'); Laeufe = @((Lauf-B8))
    OhneBasislauf = 'ohne Umbau nicht moeglich - Teil 0 steht im Testbinary des Etappenstands (Gegenprobe PluginProcessor.cpp:151 und PipeClient.cpp:221 am Basis-SHA in der Matrixzeile).'
    Erwartet = @((E 'FEHLER\s+M-74 m74_testbau_oeffnet_die_produktions_pipe_nie/namenswahl' 'm74 .../namenswahl'), (E 'Abbruch vor dem ersten Prozessor' 'Abbruch vor dem ersten Prozessor'))
    Hinweis = 'Unter der Mutation endet B8 nach Teil 0 (a) mit Exit 1, bevor ein Prozessor entsteht - kein Pipezugriff. Deshalb laufen die uebrigen Pruefungen des Beins unter dieser Mutation nicht (0 weiter gruen ist hier gewollt). Gebaut wird nur EqCopLebenslaufTest.' }

Spec @{ Zeile = 'M-74'; Teil = '(c) ein Ziel aus der Testschalter-Schleife genommen'; Datei = 'eq-copilot/plugin/CMakeLists.txt'
    Bein = 'B8 EqCopLebenslaufTest (eq-copilot/plugin/tests/LebenslaufTestMain.cpp, Teil 0)'
    Zusage = 'Jedes Ziel mit den Prozessorquellen ausser EqCopilot traegt den Testschalter NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3 (Quelltextwache ueber CMakeLists.txt).'
    Ersetzungen = @(@{
Alt = @'
             EqCopSonde014AssistentTest EqCopBriefkastenTest)
    target_compile_definitions(${ziel} PRIVATE
        NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
'@
Neu = @'
             EqCopBriefkastenTest) # MUTATION: EqCopSonde014AssistentTest aus der Testschalter-Schleife genommen
    target_compile_definitions(${ziel} PRIVATE
        NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
'@ })
    BauGrund = 'B8 liest CMakeLists.txt zur Laufzeit (__FILE__). Ein Bau unter dieser Mutation konfigurierte den Baum neu und naehme EqCopSonde014AssistentTest den Testschalter - genau die Gefahr, die M-74 misst. B8 laeuft auf dem unveraenderten Binary.'
    NachBauGrund = 'wie unter Mutation; der naechste Bau konfiguriert wegen des neuen Zeitstempels einmal neu (gleicher Inhalt).'
    Laeufe = @((Lauf-B8)); OhneBasislauf = 'ohne Umbau nicht moeglich - Teil 0 steht im Testbinary des Etappenstands.'
    Erwartet = @((E 'FEHLER\s+M-74 m74_testbau_oeffnet_die_produktions_pipe_nie/cmake' 'm74 .../cmake')) }

# --- M-30 bis M-40: Laufzeit-Arm beendet nur Eigenes (A35, laufzeit.ps1) ---
$LZ = 'tools/fl/laufzeit.ps1'
Spec @{ Zeile = 'M-30'; Teil = 'Besitzpruefung durch den Titeltest ersetzt'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Das Titel-Doppel bleibt unberuehrt: Exit 0 UEBERSPRUNGEN, der Grund nennt PID, Titel und "nicht von diesem Runner gestartet"; kein beende <pid>.'
    Ersetzungen = @(@{
Alt = @'
    if (-not $eintrag) { return @{ Eigen = $false; Grund = 'nicht von diesem Runner gestartet' } }
'@
Neu = @'
    if (-not $eintrag) { if ($inst.Fenster -and ([string]$inst.Titel) -like "*$($K.ProjektName)*") { return @{ Eigen = $true; Grund = 'eigen'; Zweck = 'fl' } }; return @{ Eigen = $false; Grund = 'nicht von diesem Runner gestartet' } }  # MUTATION: Titeltest statt Besitz (Ist-Diagnose am Basis-SHA)
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-30'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-30 .*\(a\) anderer Ordner: Titel-Doppel beendet' '309/M-30 (a) anderer Ordner: Titel-Doppel beendet'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-3[1-9]' = 'dieselbe Regel R-309-1'': ein FL mit Nakama-Diagnose.flp im Titel ohne gueltigen Besitzeintrag ist fremd - jeder Fall mit einem solchen Prozess faellt mit (Titel-Doppel, PID-Wiederverwendung, unlesbare Liste, Rueckweg)' } }

Spec @{ Zeile = 'M-31'; Teil = '-Beenden ueber den Titeltest'; Datei = $LZ; Bein = $A35Bein
    Zusage = '-Beenden und der Neustart beenden nur Eigenes: das selbst gestartete Diagnose-FL wird beendet, das Titel-Doppel nicht.'
    Ersetzungen = @(@{
Alt = @'
        if ($K.Beenden) { Beende-Diagnose-FL }
'@
Neu = @'
        if ($K.Beenden) { foreach ($i in @(FL-Instanzen | Where-Object { $_.Fenster -and ([string]$_.Titel) -like "*$($K.ProjektName)*" })) { & $script:U.BeendeProzess $i.Id } }  # MUTATION: -Beenden ueber den Titeltest (Stand am Basis-SHA)
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-31'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-31 .*-Beenden hat das Titel-Doppel beendet' '309/M-31 (a) -Beenden hat das Titel-Doppel beendet'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-38 ' = 'M-38 (b): der Austrag haengt am Beenden ueber Beende-Diagnose-FL; der alte Weg traegt nicht aus' } }

Spec @{ Zeile = 'M-32'; Teil = 'Besitzpruefung liefert immer fremd'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Das eigene Diagnose-FL wird beendet (nie gespeichert), danach wird sein Eintrag entfernt (Regressionswache).'
    Ersetzungen = @(@{
Alt = @'
    return @{ Eigen = $true; Grund = 'eigen'; Zweck = [string]$eintrag.Zweck }
'@
Neu = @'
    return @{ Eigen = $false; Grund = 'MUTATION: immer fremd' }  # MUTATION: Besitzpruefung liefert immer fremd
'@ })
    Laeufe = @((Lauf-Laufzeit 'frischer_start_schluessel'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - Regressionswache (am Basis-SHA ueber den Titel beendet).'
    Erwartet = @((E '^\[ROT\] M-59 frischer_start_schluessel' 'M-59 frischer_start_schluessel (kein Beenden vor dem Neustart)'))
    Mit = [ordered]@{ '.' = 'ohne Besitz ist kein FL eigen: jeder Fall, der ein eigenes FL beendet, neu startet oder als laufend erkennt, faellt mit derselben Regel' } }

Spec @{ Zeile = 'M-33'; Teil = 'Beende-Restprozesse ueberspringt auch Eigenes'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Der eigene Restprozess wird beendet; der nicht beendbare wird geloggt, und der Lauf geht weiter (Regressionswache).'
    Ersetzungen = @(@{
Alt = @'
        if (-not (Beende-Eigenen $i 'FL-Restprozess')) { continue }
'@
Neu = @'
        continue  # MUTATION: Beende-Restprozesse ueberspringt auch eigene Prozesse
'@ })
    Laeufe = @((Lauf-Laufzeit 'loopmidi_und_restprozesse'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E '^\[ROT\] M-07 loopmidi_und_restprozesse' 'M-07 loopmidi_und_restprozesse, Teilfall Restprozesse'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-34 .*eigenen Prozess nicht beendet' = 'derselbe Aufruf: Beende-Restprozesse beendet auch im Teilfall "Beende-Restprozesse beendet nur Eigenes" den eigenen Prozess nicht' } }

Spec @{ Zeile = 'M-34'; Teil = '(1) Besitzpruefung in Beende-Restprozesse entfernt'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Ein fensterloser FL-Prozess ohne Besitzeintrag wird nie beendet: Exit 0 UEBERSPRUNGEN "fremder FL-Prozess ohne Fenster: PID ..."; neben ihm wird nicht gemessen.'
    Ersetzungen = @(@{
Alt = @'
        if (-not (Beende-Eigenen $i 'FL-Restprozess')) { continue }
'@
Neu = @'
        & $script:U.BeendeProzess $i.Id  # MUTATION: Besitzpruefung in Beende-Restprozesse entfernt
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-34'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-34 .*Beende-Restprozesse hat den fremden Prozess beendet' '309/M-34 Teilfall "Beende-Restprozesse beendet nur Eigenes": fremder Prozess beendet'))
    Hinweis = 'Die Teilfaelle vor der Installation und vor dem FL-Start bleiben unter dieser Mutation gruen: dort steht vor Beende-Restprozesse die Fremdpruefung der Aufrufstelle (Fremde-FL, Teil (2)). Den Schutz am Beenden selbst misst der dritte Teilfall, der Beende-Restprozesse direkt ruft.' }

Spec @{ Zeile = 'M-34'; Teil = '(2) fensterlose Prozesse gelten nicht als fremd'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Neben einem fremden fensterlosen FL-Prozess wird weder installiert noch gemessen: Exit 0 UEBERSPRUNGEN mit dem Grund "fremder FL-Prozess ohne Fenster".'
    Ersetzungen = @(@{
Alt = @'
function Fremde-FL { return @(FL-Instanzen | Where-Object { Ist-Fremd $_ }) }
'@
Neu = @'
function Fremde-FL { return @(FL-Instanzen | Where-Object { $_.Fenster -and (Ist-Fremd $_) }) }  # MUTATION: fensterlose Prozesse gelten nicht als fremd (Stand am Basis-SHA)
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-34'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-34 .*Installation neben einem fremden FL-Prozess' '309/M-34 vor der Installation: installiert neben dem fremden Prozess'), (E '^\[ROT\] 309/M-34 .*FL daneben gestartet' '309/M-34 vor dem FL-Start: FL daneben gestartet'))
    Hinweis = 'Der Prozess selbst bleibt auch unter dieser Mutation unbeendet (Schutz am Beenden, Teil (1)); die Zusage "nicht messen, nicht installieren" haengt an der Fremdpruefung der Aufrufstelle.' }

Spec @{ Zeile = 'M-35'; Teil = 'Vergleich nur ueber die PID'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'PID allein ist kein Besitz: weicht Startzeit oder Befehlszeile ab, ist der Prozess fremd und wird nie beendet; der veraltete Eintrag wird beim Laufstart entfernt und protokolliert.'
    Ersetzungen = @(@{
Alt = @'
        if ($info -and $info.Startzeit -and [long]$info.Startzeit -eq $e.Startzeit -and [string]$info.Befehlszeile -eq $e.Befehlszeile) { $bleibt += $e; continue }
'@
Neu = @'
        if ($info) { $bleibt += $e; continue }  # MUTATION: Vergleich nur ueber die PID
'@ }, @{
Alt = @'
    $eintrag = @(@(Lies-Besitz) | Where-Object { $_.Pid -eq $inst.Id -and $_.Startzeit -eq [long]$info.Startzeit -and $_.Befehlszeile -eq [string]$info.Befehlszeile }) | Select-Object -First 1
'@
Neu = @'
    $eintrag = @(@(Lies-Besitz) | Where-Object { $_.Pid -eq $inst.Id }) | Select-Object -First 1  # MUTATION: Vergleich nur ueber die PID
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-35'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basis-SHA gibt es keine Besitzliste).'
    Erwartet = @((E '^\[ROT\] 309/M-35 .*\(a\) die PID allein als Besitz gewertet: beendet' '309/M-35 (a) gleiche PID, andere Startzeit: beendet'), (E '^\[ROT\] 309/M-35 .*\(b\) beendet trotz anderer Befehlszeile' '309/M-35 (b) gleiche PID und Startzeit, andere Befehlszeile: beendet'))
    Hinweis = 'Die Mutation steht an beiden Vergleichen, die die Zusage tragen: der Bereinigung beim Laufstart und dem Besitzurteil an jedem Beenden. Teilfall (c) (Befehlszeile nicht lesbar) bleibt gruen - er endet fail-closed vor dem Vergleich.' }

Spec @{ Zeile = 'M-36'; Teil = 'Besitz allein genuegt'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Eigenes FL mit fremdem Projekt: fehlt die zweite Sicherheit, gilt uebersprungen - "eigenes FL zeigt ein fremdes Projekt", nie beendet (Regressionswache).'
    Ersetzungen = @(@{
Alt = @'
    if ($eintrag.Zweck -eq 'fl' -and $inst.Fenster -and $inst.Titel -and -not ([string]$inst.Titel).StartsWith(([IO.Path]::GetFileName($projekt) + ' - '), [StringComparison]::OrdinalIgnoreCase)) {
'@
Neu = @'
    if ($false) {  # MUTATION: Besitz allein genuegt (zweite Sicherheit entfernt)
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-36'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - Regressionswache (am Basis-SHA gilt der Prozess ueber den Titel als fremd).'
    Erwartet = @((E '^\[ROT\] 309/M-36 .*eigenes FL mit fremdem Projekt beendet' '309/M-36 eigenes FL mit fremdem Projekt beendet')) }

Spec @{ Zeile = 'M-37'; Teil = '(1) erste Titeluebereinstimmung als Diagnose-PID'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Die Diagnose-PID ist nur die selbst gestartete; laeuft beim Ping ein fremdes FL mit, endet der Lauf mit Exit 0 UEBERSPRUNGEN, und die fremde PID erscheint nie als --diagnose-pid.'
    Ersetzungen = @(@{
Alt = @'
            $fremd = @($alle | Where-Object { Ist-Fremd $_ })
            if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' ('fremdes FL beim Ping: ' + (Fremd-Text $fremd[0])) }
            $inst = @($alle | Where-Object { $_.Id -eq $K.DiagnosePid -and (Ist-Eigenes-Diagnose-FL $_) -and ([string]$_.Titel).StartsWith("$($K.ProjektName) - ", [StringComparison]::OrdinalIgnoreCase) })
'@
Neu = @'
            $inst = @($alle | Where-Object { $_.Fenster -and ([string]$_.Titel) -like "*$($K.ProjektName)*" })  # MUTATION: erste Titeluebereinstimmung (Stand am Basis-SHA)
            if ($inst.Count -gt 0) { $K.DiagnosePid = $inst[0].Id }  # MUTATION
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-37'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-37 .*die fremde PID erscheint als --diagnose-pid' '309/M-37 die fremde PID erscheint als --diagnose-pid'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-3[0-9] ' = 'dieselbe Regel R-309-1'': ohne die Pruefung beim Ping misst der Lauf neben einem fremden FL' } }

Spec @{ Zeile = 'M-37'; Teil = '(2) Uebersprung bei fremdem FL beim Ping entfernt'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Laeuft beim Ping ein fremdes FL mit, endet der Lauf mit Exit 0 UEBERSPRUNGEN; neben ihm wird kein Szenario gefahren.'
    Ersetzungen = @(@{
Alt = @'
            if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' ('fremdes FL beim Ping: ' + (Fremd-Text $fremd[0])) }
'@
Neu = @'
            # MUTATION: Uebersprung bei fremdem FL beim Ping entfernt
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-37'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-37 .*Szenario neben dem fremden FL gefahren' '309/M-37 Szenario neben dem fremden FL gefahren'))
    Hinweis = 'Die Diagnose-PID bleibt unter dieser Mutation die selbst gestartete (die zweite Haelfte der Zusage, Teil (1)).' }

Spec @{ Zeile = 'M-38'; Teil = '(d) keine Bereinigung beim Laufstart'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Die Liste haelt genau die lebenden Prozesse, die dieser Runner gestartet hat: ein Eintrag ohne lebenden Prozess wird beim Laufstart entfernt.'
    Ersetzungen = @(@{
Alt = @'
        Log "Laufzeit-Arm $($K.Ticket) auf $($K.Head) (Basis $($K.Basis)), Repo $($K.Repo)"
        Bereinige-Besitz
'@
Neu = @'
        Log "Laufzeit-Arm $($K.Ticket) auf $($K.Head) (Basis $($K.Basis)), Repo $($K.Repo)"
        # MUTATION: Bereinige-Besitz beim Laufstart entfernt
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-38'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basis-SHA gibt es keine Besitzliste).'
    Erwartet = @((E '^\[ROT\] 309/M-38 .*\(d\) toter Eintrag beim Laufstart nicht entfernt' '309/M-38 (d) toter Eintrag bleibt'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-35 ' = 'dieselbe Bereinigung protokolliert die veralteten Eintraege aus M-35 (a) und (b)' } }

Spec @{ Zeile = 'M-38'; Teil = '(e) unlesbare Liste gilt als alles eigen'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Eine unlesbare Liste gilt als leer und wird protokolliert, nie als "alles eigen".'
    Ersetzungen = @(@{
Alt = @'
    if (-not $eintrag) { return @{ Eigen = $false; Grund = 'nicht von diesem Runner gestartet' } }
'@
Neu = @'
    if (-not $eintrag) { if (@($K.BesitzMeldungen.Keys | Where-Object { $_ -match 'unlesbar|nicht lesbar' }).Count -gt 0) { return @{ Eigen = $true; Grund = 'eigen'; Zweck = 'fl' } }; return @{ Eigen = $false; Grund = 'nicht von diesem Runner gestartet' } }  # MUTATION: unlesbare Liste gilt als alles eigen
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-38'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar.'
    Erwartet = @((E '^\[ROT\] 309/M-38 .*\(e\) unlesbare Liste als alles eigen gewertet' '309/M-38 (e) Titel-Doppel bei unlesbarer Liste beendet'))
    Hinweis = 'Die Mutation steht am Besitzurteil: eine synthetische Liste mit allen Prozessen allein genuegte nicht, weil die Befehlszeile des Titel-Doppels kein Projekt der Arbeitskopie nennt (zweites Merkmal). "Alles eigen" heisst hier: jeder Prozess ohne Eintrag gilt als eigen, sobald die Liste als unlesbar gemeldet ist.' }

Spec @{ Zeile = 'M-39'; Teil = 'Rueckweg auf dem alten Weg ohne Besitzpruefung'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Der Rueckweg beendet nur Eigenes; laeuft danach noch ein FL, verweigert er mit PID und Titel; ein fremdes FL wird nie beendet, um den Rueckweg zu ermoeglichen.'
    Ersetzungen = @(@{
Alt = @'
    Log 'Rueckweg nach ABWEICHUNG: (1) eigenes Diagnose-FL beenden (nie speichern) und eigene fensterlose Restprozesse'
    Beende-Diagnose-FL
    Beende-Restprozesse
'@
Neu = @'
    Log 'Rueckweg nach ABWEICHUNG: (1) eigenes Diagnose-FL beenden (nie speichern) und eigene fensterlose Restprozesse'
    foreach ($i in @(FL-Instanzen | Where-Object { ($_.Fenster -and ([string]$_.Titel) -like "*$($script:K.ProjektName)*") -or -not $_.Fenster })) { & $script:U.BeendeProzess $i.Id }  # MUTATION: alter Weg ohne Besitzpruefung (Stand am Basis-SHA)
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-39'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-39 .*der Rueckweg hat das Titel-Doppel beendet' '309/M-39 (b) der Rueckweg hat das Titel-Doppel beendet')) }

Spec @{ Zeile = 'M-40'; Teil = 'Uebersprung eines fremden FL mit Exit 3'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Der Runner-Vertrag bleibt: die Uebersprunge aus M-30, M-31, M-34, M-36 und M-37 enden mit 0 UEBERSPRUNGEN (Regressionswache).'
    Ersetzungen = @(@{
Alt = @'
            if ($inst.Ueberspringen) { Ende 0 'UEBERSPRUNGEN' $inst.Grund }
'@
Neu = @'
            if ($inst.Ueberspringen) { Ende 3 'VORAUSSETZUNG' $inst.Grund }  # MUTATION: Uebersprung eines fremden FL mit Exit 3
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-30'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E '^\[ROT\] 309/M-30 .*\(a\) anderer Ordner: Exit 3 VORAUSSETZUNG' '309/M-30 (a) verlangt Exit 0, ist Exit 3'))
    Mit = [ordered]@{ '.' = 'derselbe Uebersprung vor der Installation (NAK-286 M-03, R-286-5): jeder Fall mit fremdem FL vor der Installation verlangt Exit 0' } }

# --- M-41 bis M-51: Antwortzuordnung und Importweg (A35, szenario.py und Szenariodateien) ---
$SZ = 'tools/fl/szenario.py'
Spec @{ Zeile = 'M-41'; Teil = 'Kennungsvergleich invertiert'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Zugeordnet -> gemessen: eine Antwort mit der gesendeten Kennung wird bewertet wie bisher; die Rohzeile nennt die Kennung.'
    Ersetzungen = @(@{
Alt = @'
    if erhalten == kennung:
'@
Neu = @'
    if erhalten != kennung:  # MUTATION: Kennungsvergleich invertiert
'@ })
    Laeufe = @((Lauf-Szenario '309/M-41'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basis-SHA keine Kennung).'
    Erwartet = @((E '^\[ROT\] 309/M-41 ' '309/M-41 antwort_mit_kennung_gemessen'))
    Mit = [ordered]@{ '.' = 'derselbe Vergleich (_zuordnen) ordnet jede MCP-Antwort des Szenariopfads zu' } }

Spec @{ Zeile = 'M-42'; Teil = 'Kennungspruefung entfernt'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Antwort mit fremder oder ohne Kennung -> keine Messung: UNGEMESSEN, gezaehlt, Szenario-Exit 5, nie 0; ein Ping ohne Kennung ist Exit 3.'
    Ersetzungen = @(@{
Alt = @'
    if erhalten == kennung:
'@
Neu = @'
    if True:  # MUTATION: Kennungspruefung entfernt
'@ })
    Laeufe = @((Lauf-Szenario '309/M-42'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-42 ' '309/M-42 fremde_oder_fehlende_kennung_ungemessen'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-4[46] ' = 'dieselbe Zuordnung: ohne Kennungspruefung wird auch die verspaetete Antwort gemessen' } }

Spec @{ Zeile = 'M-43'; Teil = 'Echo-Erwartung aus bereitschaft.json genommen'; Datei = 'docs/gesundheit/szenarien/bereitschaft.json'; Bein = $A35Bein
    Zusage = 'Echo abweichend -> verfehlt: der zweite mixer.getPeaks-Schritt der echten bereitschaft.json mit track 1 statt 0 ist verfehlt.'
    Ersetzungen = @(@{
Alt = @'
  {"aktion": "mixer.getPeaks", "params": {"track": 0}, "erwarte": {"track": {"gleich": 0}, "peak_max": {"min": 0.01, "max": 1.0}}},
'@
Neu = @'
  {"aktion": "mixer.getPeaks", "params": {"track": 0}, "erwarte": {"peak_max": {"min": 0.01, "max": 1.0}}},
'@ })
    Laeufe = @((Lauf-Szenario '309/M-43'), (Lauf-A35))
    OhneBasislauf = 'ohne Umbau nicht moeglich - der Fall liest die Szenariodatei des Etappenstands; am Basis-SHA fehlt die Echo-Erwartung (Gegenprobe bereitschaft.json:16-17 in der Matrixzeile).'
    Erwartet = @((E '^\[ROT\] 309/M-43 ' '309/M-43 echo_abweichung_verfehlt'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-48 ' = 'dieselbe Echo-Erwartung: M-48 prueft jeden Echo-Schritt jeder Szenariodatei' } }

Spec @{ Zeile = 'M-44'; Teil = 'Kennungsvergleich entfernt'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Keine Kette: nach zwei Timeouts wird B nie an der verspaeteten Antwort von A gemessen (UNGEMESSEN, gezaehlt); C ist wieder gemessen; Szenario-Exit 5.'
    Ersetzungen = @(@{
Alt = @'
    if erhalten == kennung:
'@
Neu = @'
    if True:  # MUTATION: Kennungsvergleich entfernt
'@ })
    Laeufe = @((Lauf-Szenario '309/M-44'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-44 .*B an der verspaeteten Antwort von A gemessen' '309/M-44 B an der verspaeteten Antwort von A gemessen'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-4[26] ' = 'dieselbe Zuordnung: ohne Kennungsvergleich wird jede Antwort gemessen' } }

Spec @{ Zeile = 'M-46'; Teil = '(a) Wiederholung in einer Schleife'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Genau eine Wiederholung mit derselben Frist nach 0,5 s Pause (NAK-286 M-78; Regressionswache).'
    Ersetzungen = @(@{
Alt = @'
    if not antwort.get("_ungemessen") and not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):
'@
Neu = @'
    while not antwort.get("_ungemessen") and not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):  # MUTATION: Wiederholung in einer Schleife bis zum Erfolg
'@ })
    Laeufe = @((Lauf-Szenario 'timeout_genau_einmal'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E '^\[ROT\] M-78 timeout_genau_einmal: .*\(b\)' 'M-78 timeout_genau_einmal (b): drei Aufrufe statt genau zwei'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-44 ' = 'Schritt A laeuft unter der Schleife ein drittes Mal und bekommt die verspaetete Antwort von A statt eines Timeouts' } }

Spec @{ Zeile = 'M-46'; Teil = '(b) Wiederholung mit derselben Kennung'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Die Wiederholung traegt eine neue Kennung; eine verspaetete Antwort auf den ersten Versuch zaehlt nicht als Antwort der Wiederholung.'
    Ersetzungen = @(@{
Alt = @'
        kennung = umg.mcp_kennung()
'@
Neu = @'
        kennung = einmal.__dict__.setdefault("kennung", umg.mcp_kennung())  # MUTATION: die Wiederholung traegt dieselbe Kennung
'@ })
    Laeufe = @((Lauf-Szenario '309/M-46'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basis-SHA keine Kennung).'
    Erwartet = @((E '^\[ROT\] 309/M-46 .*Wiederholung ohne neue Kennung' '309/M-46 Wiederholung ohne neue Kennung'), (E '^\[ROT\] 309/M-46 .*als Antwort der Wiederholung gewertet' '309/M-46 verspaetete Antwort des ersten Versuchs als Antwort der Wiederholung gewertet'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-44 ' = 'dieselbe Kennung bei der Wiederholung von A: die verspaetete Antwort von A passt nun zur Kennung des naechsten Versuchs' } }

Spec @{ Zeile = 'M-48'; Teil = 'Echo-Erwartung aus einem Schritt genommen'; Datei = 'docs/gesundheit/szenarien/snapshot-runde01.json'; Bein = $A35Bein
    Zusage = 'Die Szenariodateien fragen das Echo dort ab, wo die Aufnahme vom 13.09.2026 es traegt (track, index, mode, mit gleich auf den gesendeten Wert).'
    Ersetzungen = @(@{
Alt = @'
  {"aktion": "transport.setLoopMode", "params": {"mode": "song"}, "erwarte": {"mode": {"gleich": "song"}}},
'@
Neu = @'
  {"aktion": "transport.setLoopMode", "params": {"mode": "song"}},
'@ })
    Laeufe = @((Lauf-Szenario '309/M-48'), (Lauf-A35)); OhneBasislauf = 'ohne Umbau nicht moeglich - der Fall liest die Szenariodateien des Etappenstands (Gegenprobe bereitschaft.json:16-17 am Basis-SHA in der Matrixzeile).'
    Erwartet = @((E '^\[ROT\] 309/M-48 .*snapshot-runde01\.json Schritt \d+ transport\.setLoopMode \(mode\)' '309/M-48 snapshot-runde01.json transport.setLoopMode (mode)')) }

Spec @{ Zeile = 'M-50'; Teil = 'Stellvertreter entfernt'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Kein Import von fl_studio_mcp.utils.fl_trigger und pynput: vor dem Import steht der Stellvertreter in sys.modules; danach prueft eine Wache sys.modules, ein Treffer ist Exit 3 mit dem Modul.'
    Ersetzungen = @(@{
Alt = @'
    if MCP_UTILS not in sys.modules:
        paket = importlib.import_module("fl_studio_mcp")
'@
Neu = @'
    if False:  # MUTATION: Stellvertreter entfernt
        paket = importlib.import_module("fl_studio_mcp")
'@ })
    Laeufe = @((Lauf-Szenario '309/M-50'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-50 .*Ping ueber das Attrappenpaket: Exit 3' '309/M-50 Ping ueber das Attrappenpaket: Exit 3 (fl_trigger geladen)'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-51 ' = 'das Erfassungsskript laedt lade_mcp_modul aus derselben Datei - derselbe Stellvertreter fehlt' } }

Spec @{ Zeile = 'M-51'; Teil = 'Stellvertreter im Erfassungsskript entfernt'; Datei = $SZ; Bein = $A35Bein
    Zusage = 'Derselbe Riegel im Unterprozess: das Erfassungsskript laedt fenster ueber denselben Stellvertreter und bricht vor der Erfassung ab, wenn fl_trigger oder pynput in sys.modules stehen.'
    Ersetzungen = @(@{
Alt = @'
                  "from szenario import lade_mcp_modul; "
                  "erfasse = lade_mcp_modul('fl_studio_mcp.utils.fenster').capture_process_window; a = sys.argv[1:6]; "
'@
Neu = @'
                  "import importlib; "  # MUTATION: Stellvertreter im Erfassungsskript entfernt
                  "erfasse = importlib.import_module('fl_studio_mcp.utils.fenster').capture_process_window; a = sys.argv[1:6]; "
'@ })
    Laeufe = @((Lauf-Szenario '309/M-51'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-51 .*fl_trigger wurde im Erfassungsskript geladen' '309/M-51 fl_trigger im Erfassungsskript geladen')) }

# --- M-52 bis M-56: MCP-Pin (A35, laufzeit.ps1) ---
Spec @{ Zeile = 'M-52'; Teil = 'Abgleich uebersprungen'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Gepinnt -> der Lauf geht weiter; das Protokoll traegt eine Zeile MCP-Stand: mit Revision, Zweig, Sauberkeit, Zahl der gepinnten Dateien und SHA-256 von uv.lock, vor Installation, setup-local.ps1 und dem ersten uv run.'
    Ersetzungen = @(@{
Alt = @'
        if (-not $lohnt.Ja) { Ende 0 'UEBERSPRUNGEN' $lohnt.Grund }
        Pruefe-McpStand
'@
Neu = @'
        if (-not $lohnt.Ja) { Ende 0 'UEBERSPRUNGEN' $lohnt.Grund }
        # MUTATION: Pruefe-McpStand uebersprungen
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-52'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basis-SHA kein Pin).'
    Erwartet = @((E '^\[ROT\] 309/M-52 .*keine Zeile MCP-Stand' '309/M-52 keine Zeile MCP-Stand'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-5[3-6] ' = 'dieselbe Pruefung entfaellt: kein Pin wird mehr geprueft' } }

Spec @{ Zeile = 'M-53'; Teil = 'Revisionsvergleich entfernt'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Revision weicht ab -> Exit 3 VORAUSSETZUNG "MCP-Stand weicht ab: Soll <rev>, Ist <rev>"; kein setup-local.ps1, kein uv run, keine Installation.'
    Ersetzungen = @(@{
Alt = @'
    if ($ist -ne $soll) { Ende 3 'VORAUSSETZUNG' "MCP-Stand weicht ab: Soll $soll, Ist $ist" }
'@
Neu = @'
    # MUTATION: Revisionsvergleich entfernt
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-53'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-53 ' '309/M-53 mcp_revision_weicht_ab'))
    Mit = [ordered]@{ '^\[ROT\] 309/M-56 ' = 'derselbe Vergleich: M-56 laeuft mit abweichender Revision und verlangt Exit 3' } }

Spec @{ Zeile = 'M-54'; Teil = 'nur der Revisionsvergleich'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Gleiche Revision, aber getrackte Datei geaendert, gepinnter Pfad fehlt oder Blob-Kennung weicht ab -> Exit 3 VORAUSSETZUNG mit Pfad und Grund.'
    Ersetzungen = @(@{
Alt = @'
    $status = & $script:U.McpGit @('status', '--porcelain', '--untracked-files=no')
    if ($status.Exit -ne 0) { Ende 3 'VORAUSSETZUNG' "MCP-Stand: git status scheitert (Exit $($status.Exit)): $((@($status.Fehler) -join ' ').Trim())" }
    $geaendert = @($status.Zeilen | Where-Object { "$_".Trim() })
    if ($geaendert.Count -gt 0) {
        Ende 3 'VORAUSSETZUNG' ("MCP-Stand weicht ab: getrackte Datei geaendert: {0}{1}" -f "$($geaendert[0])".Trim(), $(if ($geaendert.Count -gt 1) { " (und $($geaendert.Count - 1) weitere)" } else { '' }))
    }
    foreach ($pfad in @($dateien.Keys | Sort-Object)) {
        $blob = & $script:U.McpGit @('rev-parse', "HEAD:$pfad")
        $istBlob = Erste-Zeile $blob
        if ($blob.Exit -ne 0 -or -not $istBlob) { Ende 3 'VORAUSSETZUNG' "MCP-Stand weicht ab: $pfad fehlt im Commit $ist" }
        if ($istBlob -ne [string]$dateien[$pfad]) { Ende 3 'VORAUSSETZUNG' "MCP-Stand weicht ab: $pfad Blob Soll $($dateien[$pfad]), Ist $istBlob (Pin-Datei in sich widerspruechlich)" }
    }
'@
Neu = @'
    # MUTATION: nur der Revisionsvergleich - Sauberkeit und Blob-Kennungen ungeprueft
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-54'), (Lauf-A35)); OhneBasislauf = $A35HeuteRot
    Erwartet = @((E '^\[ROT\] 309/M-54 .*\(a\) Exit' '309/M-54 (a) getrackte Datei geaendert: Lauf geht weiter'), (E '^\[ROT\] 309/M-54 .*\(b\) Exit' '309/M-54 (b) Pfad fehlt'), (E '^\[ROT\] 309/M-54 .*\(c\) Exit' '309/M-54 (c) Blob weicht ab')) }

Spec @{ Zeile = 'M-55'; Teil = 'fehlende Pin-Datei heisst kein Pin, weiter'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Pin-Datei fehlt, ist kein JSON, traegt ein fremdes format, oder git scheitert -> Exit 3, nie ein Lauf ohne geprueften Pin.'
    Ersetzungen = @(@{
Alt = @'
    if (-not $text) { Ende 3 'VORAUSSETZUNG' "MCP-Stand: Pin-Datei fehlt ($($K.McpStand))" }
'@
Neu = @'
    if (-not $text) { Log 'MCP-Stand: kein Pin, weiter'; return }  # MUTATION: fehlende Pin-Datei heisst kein Pin, weiter
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-55'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar.'
    Erwartet = @((E '^\[ROT\] 309/M-55 .*fehlt: Exit' '309/M-55 Pin-Datei fehlt: Lauf geht weiter')) }

Spec @{ Zeile = 'M-56'; Teil = 'Pinpruefung schreibt den Iststand zurueck'; Datei = $LZ; Bein = $A35Bein
    Zusage = 'Nur ein datierter Nakama-Commit aendert den Pin: der Runner schreibt tools/fl/mcp-stand.json nie (nach einem Lauf mit abweichender Revision sind ihre Bytes unveraendert).'
    Ersetzungen = @(@{
Alt = @'
    if ($ist -ne $soll) { Ende 3 'VORAUSSETZUNG' "MCP-Stand weicht ab: Soll $soll, Ist $ist" }
'@
Neu = @'
    if ($ist -ne $soll) { & $script:U.SchreibeText $K.McpStand ($text -replace $soll, $ist); Ende 3 'VORAUSSETZUNG' "MCP-Stand weicht ab: Soll $soll, Ist $ist" }  # MUTATION: die Pinpruefung schreibt den Iststand zurueck
'@ })
    Laeufe = @((Lauf-Laufzeit '309/M-56'), (Lauf-A35))
    OhneBasislauf = 'entfaellt - heute nicht messbar.'
    Erwartet = @((E '^\[ROT\] 309/M-56 .*abweichend: Exit 3, Pin [0-9A-F]{64} -> [0-9A-F]{64}' '309/M-56 Hash der Pin-Datei aendert sich'))
    Hinweis = 'Die Pin-Datei des Selbsttests ist die Kopie in der Testwurzel ($script:T.Repo\tools\fl\mcp-stand.json); die Mutation schreibt nie in das Repo.' }

# --- M-45, M-47, M-49: Nachbarrepo fl-studio-mcp (pytest) ---
$MC = 'src/fl_studio_mcp/utils/midi_connection.py'
$PytestBein = 'pytest im Nachbarrepo fl-studio-mcp (tests/test_request_id.py; dazu die ganze Suite)'
Spec @{ Zeile = 'M-45'; Teil = 'Uebergehen fremder Kennungen entfernt'; Repo = 'mcp'; Datei = $MC; Bein = $PytestBein
    Zusage = 'Zwei verspaetete Antworten fremder Kennung werden uebergangen, ohne die Datei zu loeschen, und je Kennung einmal gezaehlt; zurueck kommt die eigene Antwort.'
    Ersetzungen = @(@{
Alt = @'
                if request_id is not None and answer_id is not None and answer_id != request_id:
'@
Neu = @'
                if False:  # MUTATION: Uebergehen fremder Kennungen entfernt
'@ })
    Laeufe = @((Lauf-Pytest @('tests/test_request_id.py::test_zwei_spaete_antworten')), (Lauf-Pytest @()))
    Basislauf = 'test_zwei_spaete_antworten'
    Erwartet = @((E 'test_zwei_spaete_antworten FAILED' 'tests/test_request_id.py::test_zwei_spaete_antworten'))
    Mit = [ordered]@{ 'test_frist_absolut' = 'dieselbe Uebergehen-Regel: ohne sie endet die Wartezeit an der ersten fremden Antwort' } }

Spec @{ Zeile = 'M-47'; Teil = 'Frist nach jeder uebergangenen Antwort neu gestartet'; Repo = 'mcp'; Datei = $MC; Bein = $PytestBein
    Zusage = 'Die Frist bleibt absolut ab dem Senden: uebergangene Antworten verlaengern sie nie; nach Ablauf kommt ein Timeout, der die Zahl verworfener Antworten nennt.'
    Ersetzungen = @(@{
Alt = @'
                    time.sleep(poll_interval)
                    continue
'@
Neu = @'
                    deadline = time.time() + timeout  # MUTATION: Frist nach jeder uebergangenen Antwort neu gestartet
                    time.sleep(poll_interval)
                    continue
'@ })
    Laeufe = @((Lauf-Pytest @('tests/test_request_id.py::test_frist_absolut')), (Lauf-Pytest @()))
    OhneBasislauf = 'entfaellt - heute nicht messbar (am Basisstand 01f72386 kennt _wait_for_response keine Kennung).'
    Erwartet = @((E 'test_frist_absolut FAILED' 'tests/test_request_id.py::test_frist_absolut'))
    Hinweis = 'Die Uhr-Attrappe des Falls hat seit dem zweiten Nachbarcommit dieser Etappe eine Obergrenze (stop_after, 10 s nach dem Start). Im ersten Rotbeweislauf am 19.09.2026 um 01:42 Uhr lief der Fall unter dieser Mutation endlos: die fremde Antwortdatei bleibt liegen, jede Abfrage setzte die Frist neu, und die Uhr-Attrappe rueckt nur beim Warten vor. Eine verlaengerte Frist endet jetzt als Fehlschlag statt als Haenger.' }

Spec @{ Zeile = 'M-49'; Teil = 'Kopierzeile im Controller entfernt'; Repo = 'mcp'; Datei = 'fl_controller/device_FLStudioMCP.py'; Bein = $PytestBein
    Zusage = 'Ein Commit im Nachbarrepo fuehrt die Kennung durch: der Controller kopiert request_id in jede Antwort, Erfolg und Fehler.'
    Ersetzungen = @(@{
Alt = @'
        response["request_id"] = request_id
'@
Neu = @'
        pass  # MUTATION: Kopierzeile entfernt
'@ })
    Laeufe = @((Lauf-Pytest @('tests/test_request_id.py::test_controller_kopiert_kennung_in_erfolg_und_fehler')), (Lauf-Pytest @()))
    Basislauf = 'test_controller_kopiert_kennung_in_erfolg_und_fehler'
    Erwartet = @((E 'test_controller_kopiert_kennung_in_erfolg_und_fehler FAILED' 'tests/test_request_id.py::test_controller_kopiert_kennung_in_erfolg_und_fehler')) }

# ---------------------------------------------------------------- Handlauf M-29
function Fahre-Handlauf {
    $z = New-Object System.Collections.Generic.List[string]
    $kopf = (& git -C $Repo rev-parse --short=8 HEAD).Trim()
    $z.Add('NAK-309 Etappe 3 - Handlauf M-29 (T3-13-01, NB-1): der dokumentierte Ablauf eq-copilot/README.md')
    $z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}" -f (Get-Date), $kopf))
    $z.Add('Zusage: Der dokumentierte Ablauf endet mit PROBE OK v2 - KONFLIKT OK - KONFLIKT-ENDE OK (Exit 0): das Beispiel nennt das Server-Binary der Probe als drittes Argument.')
    $z.Add('Rotbeweis (R-309-8): derselbe Aufruf ohne drittes Argument nach dem Bau, erwartet Exit 1 (Servererwartung = installierter Broker, Stufe 1 scheitert an der Dateiidentitaet).')
    $z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Handlauf (SHA-256 $WerkzeugSha)")
    $z.Add('Nur Probe-Namen: \\.\pipe\evenacadia.eq-copilot.m2probe (eqcop-broker-probe.exe ohne Pipeargument); keine Produktions-Pipe, kein FL.')
    $z.Add('')
    $z.Add('--- BAU ---')
    $bauGut = Baue @('EqCopPipeProbe') $null $z
    $cargo = (Get-Command cargo).Source
    $c = Fahre $cargo @('build', '--release', '--bin', 'eqcop-broker-probe', '--manifest-path', 'broker/Cargo.toml', '--color', 'never') $Repo 1800
    $z.Add("Aufruf: cargo build --release --bin eqcop-broker-probe --manifest-path broker/Cargo.toml --color never")
    $z.Add("Exitcode: $($c.Code)")
    foreach ($l in @(($c.Fehler -split "`r?`n") | Where-Object { $_ -match 'Finished|Compiling eqcop|error' } | Select-Object -First 6)) { $z.Add("  " + $l.Trim()) }
    $broker = Join-Path $Repo 'broker\target\release\eqcop-broker-probe.exe'
    $z.Add(("eqcop-broker-probe.exe {0:yyyy-MM-dd HH:mm:ss.fff}, SHA-256 {1}" -f (Get-Item -LiteralPath $broker).LastWriteTime, (Sha $broker)))
    if (-not $bauGut -or $c.Code -ne 0) { $z.Add('Bau gescheitert - kein Handlauf.') }
    else {
        $ordner = Join-Path $Repo 'eq-copilot'
        foreach ($fall in @(
            @{ Name = 'Lauf 1 (README, mit drittem Argument)'; Args = @('\\.\pipe\evenacadia.eq-copilot.m2probe', '5', '..\broker\target\release\eqcop-broker-probe.exe'); Erwartet = 0 },
            @{ Name = 'Lauf 2 (Rotbeweis R-309-8, ohne drittes Argument)'; Args = @('\\.\pipe\evenacadia.eq-copilot.m2probe', '5'); Erwartet = 1 })) {
            $z.Add('')
            $z.Add("--- $($fall.Name) ---")
            $bAus = Join-Path $Temp 'broker.out'; $bFeh = Join-Path $Temp 'broker.err'
            $b = Start-Process -FilePath $broker -ArgumentList '30' -WorkingDirectory $ordner -NoNewWindow -PassThru -RedirectStandardOutput $bAus -RedirectStandardError $bFeh
            $null = $b.Handle
            $z.Add('Terminal 1: ..\broker\target\release\eqcop-broker-probe.exe 30   (Arbeitsordner eq-copilot, PID ' + $b.Id + ')')
            $t0 = Get-Date
            while (-not (@([IO.Directory]::GetFiles('\\.\pipe\') | Where-Object { $_ -like '*evenacadia.eq-copilot.m2probe' }).Count) -and ((Get-Date) - $t0).TotalSeconds -lt 15) { Start-Sleep -Milliseconds 200 }
            $r = Fahre $Bin.Probe $fall.Args $ordner 120
            $z.Add('Terminal 2: build\plugin\EqCopPipeProbe_artefacts\Release\EqCopPipeProbe.exe ' + (($fall.Args | ForEach-Object { if ($_ -like '\\*') { '"' + $_ + '"' } else { $_ } }) -join ' '))
            $z.Add("Exitcode: $($r.Code) (erwartet $($fall.Erwartet)) -> $(if ("$($r.Code)" -eq "$($fall.Erwartet)") { 'wie erwartet' } else { 'ABWEICHEND' })")
            foreach ($l in @(($r.Aus + "`n" + $r.Fehler) -split "`r?`n" | Where-Object { $_.Trim() })) { $z.Add("  $l") }
            if (-not $b.WaitForExit(60000)) { try { $b.Kill($true) } catch { } }
            $bText = $(if (Test-Path -LiteralPath $bFeh) { [IO.File]::ReadAllText($bFeh) } else { '' }) + $(if (Test-Path -LiteralPath $bAus) { [IO.File]::ReadAllText($bAus) } else { '' })
            $z.Add("Broker-Probe: Exit $($b.ExitCode)")
            foreach ($l in @($bText -split "`r?`n" | Where-Object { $_.Trim() } | Select-Object -First 8)) { $z.Add("  $l") }
            foreach ($f in @($bAus, $bFeh)) { if (Test-Path -LiteralPath $f) { [IO.File]::Delete($f) } }
        }
    }
    $roh = Join-Path $RohOrdner 'NAK-309-etappe3-pipeprobe.txt'
    [IO.File]::WriteAllText($roh, [string]::Join("`n", $z) + "`n", $Utf8)
    Write-Host "Handlauf M-29 geschrieben: $(Rel $roh)"
}

# ---------------------------------------------------------------- Einzelbeine
function Fahre-Einzelbeine {
    $z = New-Object System.Collections.Generic.List[string]
    $kopf = (& git -C $Repo rev-parse --short=8 HEAD).Trim()
    $z.Add('NAK-309 Etappe 3 - einzeln gefahrene Beine (Auftrag Schritt 4: A4b, A35, A1, B14, B8 mit M-74)')
    $z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}; Arbeitsbaum der Beinquellen gleich HEAD" -f (Get-Date), $kopf))
    $z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe3-rot.ps1 -Einzelbeine (SHA-256 $WerkzeugSha)")
    $z.Add('')
    $z.Add('--- BAU ---')
    # Bezug fuer den Zeitstempel je Ziel: die Quellen dieser Etappe, die es uebersetzt (auch nach den Ruecknahmen der
    # Rotbeweise, die LastWriteTime auf jetzt setzen).
    $null = Baue @('EqCopPipeClientTest', 'EqCopLebenslaufTest', 'EqCopNullTest', 'EqCopSonde012ProjectReloadTest') $null $z
    foreach ($bein in @(
        @{ Name = 'A4b EqCopPipeClientTest'; Lauf = (Lauf-A4b) },
        @{ Name = 'A35 selbsttest.py'; Lauf = (Lauf-A35) },
        @{ Name = 'A1 EqCopNullTest'; Lauf = @{ Exe = $Bin.A1; Args = @(); Ordner = $Repo; Art = 'cpp' } },
        @{ Name = 'B14 EqCopSonde012ProjectReloadTest'; Lauf = @{ Exe = $Bin.B14; Args = @(); Ordner = $Repo; Art = 'cpp' } },
        @{ Name = 'B8 EqCopLebenslaufTest (darin M-74)'; Lauf = (Lauf-B8) })) {
        $n = Fahre-Lauf $bein.Lauf
        $z.Add('')
        $z.Add("--- $($bein.Name) ---")
        $z.Add("Aufruf: $($n.Aufruf)")
        $z.Add("Exitcode: $($n.Code)")
        $z.Add("Pruefungen ok: $($n.Ok.Count), rot: $($n.Rot.Count)")
        foreach ($r in $n.Rot) { $z.Add("  $r") }
        foreach ($r in $n.Summe) { $z.Add("  $r") }
        if ($bein.Name -like 'B8*') { foreach ($l in @($n.Ok | Where-Object { $_ -match 'M-74' })) { $z.Add("  $l") } }
    }
    $roh = Join-Path $RohOrdner 'NAK-309-etappe3-einzelbeine.txt'
    [IO.File]::WriteAllText($roh, [string]::Join("`n", $z) + "`n", $Utf8)
    Write-Host "Einzelbeine geschrieben: $(Rel $roh)"

    $m = New-Object System.Collections.Generic.List[string]
    $mk = (& git -C $McpRepo rev-parse HEAD).Trim()
    $m.Add('NAK-309 Etappe 3 - Pytest im Nachbarrepo fl-studio-mcp (M-45, M-47, M-49; Rotbeweise in NAK-309-rot-M-45.txt, -M-47.txt, -M-49.txt)')
    $m.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, fl-studio-mcp {1} (evenacadia-local), Arbeitsbaum sauber: {2}" -f (Get-Date), $mk, $(if (-not (& git -C $McpRepo status --porcelain --untracked-files=no)) { 'ja' } else { 'NEIN' })))
    $m.Add('PYTHONDONTWRITEBYTECODE=1, -p no:cacheprovider (kein Schreiben in getrackte __pycache__-Dateien)')
    $n = Fahre-Lauf (Lauf-Pytest @())
    $m.Add("Aufruf: $($n.Aufruf)   (Arbeitsordner: MCP-Repo)")
    $m.Add("Exitcode: $($n.Code)")
    foreach ($l in @($n.Ok) + @($n.Rot) + @($n.Summe)) { $m.Add("  $l") }
    $roh = Join-Path $RohOrdner 'NAK-309-etappe3-mcp-tests.txt'
    [IO.File]::WriteAllText($roh, [string]::Join("`n", $m) + "`n", $Utf8)
    Write-Host "Pytest geschrieben: $(Rel $roh)"
}

# ---------------------------------------------------------------- Einstieg
if ($Handlauf) { $script:Cmake = Finde-CMake; Fahre-Handlauf; exit 0 }
if ($Einzelbeine) { $script:Cmake = Finde-CMake; Fahre-Einzelbeine; exit 0 }
$script:Cmake = if ($Trocken) { $null } else { Finde-CMake }
$script:Geschrieben = New-Object System.Collections.Generic.HashSet[string]
$auswahl = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$fehler = 0
foreach ($s in $S) {
    if ($auswahl.Count -gt 0 -and $auswahl -notcontains $s.Zeile) { continue }
    if (-not $s.PSObject.Properties['Repo']) { $s | Add-Member -NotePropertyName Repo -NotePropertyValue 'nakama' }
    foreach ($feld in @('Bau', 'BauGrund', 'NachBauGrund', 'Basislauf', 'OhneBasislauf', 'Mit', 'Hinweis')) {
        if (-not $s.PSObject.Properties[$feld]) { $s | Add-Member -NotePropertyName $feld -NotePropertyValue $null }
    }
    try { Rotbeweis $s }
    catch { $fehler++; Write-Host ("{0} {1}: WERKZEUGFEHLER {2}" -f $s.Zeile, $s.Teil, $_.Exception.Message) }
}
exit $(if ($fehler -gt 0) { 2 } else { 0 })
