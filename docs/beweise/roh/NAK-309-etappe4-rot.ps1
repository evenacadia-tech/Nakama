#Requires -Version 7.4
<#
NAK-309 Etappe 4 - Rotbeweise nach Paragraph 6.1 (docs/beweise/NAK-309.md), Gegenproben und Einzelbeine.

Je Matrixzeile M-57 bis M-72 und M-75 eine Mutation genau an der Zeile, die die Zusage traegt; nur das betroffene
Bein (C++: nur sein Ziel bauen, dann das Binary; Rust: cargo test mit Filter; Python: der Fall ueber --nur, dazu das
ganze Bein; A4 und A4-SI zusaetzlich im Verfahren des Runners mit Meldeordner und --nicht-gelaufen); Ruecknahme
bytegleich mit SHA-256, LastWriteTime auf jetzt, Bein danach gruen. Ergebnis je Zeile:
docs/beweise/roh/NAK-309-rot-M-NN.txt (mehrere Teile einer Zeile in derselben Datei).

Aufruf vom Workspace-Root:
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe4-rot.ps1 -Trocken        # jede Fundstelle genau einmal?
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe4-rot.ps1                 # alle Rotbeweise
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe4-rot.ps1 -Nur M-57,M-75
  pwsh -NoProfile -File docs/beweise/roh/NAK-309-etappe4-rot.ps1 -Einzelbeine    # B2, A12, B10, A4, A4-SI, B22, A36

Grenzen (Auftrag Etappe 4): kein FL, keine Installation, keine Aufgabe \Nakama\*, keine Produktions-Pipe (B10 faehrt
nur seinen eigenen Testserver auf Probe-Namen). Die Umgebungsvariablen NAKAMA_NICHT_GELAUFEN und
NAKAMA_TEST_JUNCTION_VERWEIGERN stehen nur in der Umgebung des Kindprozesses (Start-Process -Environment). Vor jeder
Mutation muss die Datei gleich HEAD sein.
#>
param(
    [string[]]$Nur = @(),
    [switch]$Trocken,
    [switch]$Einzelbeine
)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$RohOrdner = Join-Path $Repo 'docs\beweise\roh'
$Temp = Join-Path ([IO.Path]::GetTempPath()) 'nakama-nak309-e4-rot'
[void](New-Item -ItemType Directory -Force -Path $Temp)
$Utf8 = New-Object System.Text.UTF8Encoding($false)
$WerkzeugSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $PSCommandPath).Hash
$Py = (Get-Command py).Source
$Cargo = (Get-Command cargo).Source
$Pwsh = (Get-Command pwsh).Source
$Basis = '8d573e9fc3ca9b4e2bc52fe20a56afc89a9dbb0e'
$Bau = Join-Path $Repo 'eq-copilot\build'
$Bin = @{
    B2  = Join-Path $Bau 'plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe'
    B10 = Join-Path $Bau 'plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe'
    B22 = Join-Path $Bau 'plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe'
}
$Ziel = @{ B2 = 'EqCopStateMigrationTest'; B10 = 'EqCopIpcTest'; B22 = 'EqCopSonde013FingerprintGoldenTest' }
# Je Ziel die Quellen dieser Etappe, die es uebersetzt - Bezug der Zeitstempelpruefung ohne Mutation.
$EtappenQuellen = @{
    EqCopStateMigrationTest            = @('eq-copilot\plugin\tests\StateMigrationTestMain.cpp')
    EqCopIpcTest                       = @('eq-copilot\plugin\tests\IpcTestMain.cpp', 'eq-copilot\plugin\tests\V3TestServer.h')
    EqCopSonde013FingerprintGoldenTest = @('eq-copilot\plugin\tests\Sonde013FingerprintGoldenTest.cpp')
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
    if (-not (Test-Path -LiteralPath $pfad)) { return '' }
    for ($versuch = 1; $versuch -le 10; $versuch++) {
        try {
            $s = [IO.File]::Open($pfad, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete)
            try { return (New-Object IO.StreamReader($s, $Utf8)).ReadToEnd() } finally { $s.Dispose() }
        } catch { Start-Sleep -Milliseconds 500 }
    }
    return "(Ausgabe nicht lesbar: $pfad)"
}

function Fahre([string]$exe, [string[]]$argumente, [string]$ordner, [int]$frist = 1800, [hashtable]$umgebung = @{}) {
    # Umleitung in Dateien statt Pipes (kein Warten auf Enkelprozesse wie vctip.exe, NAK-300); harte Frist;
    # Umgebungsvariablen nur fuer den Kindprozess.
    $n = [guid]::NewGuid().ToString('N').Substring(0, 12)
    $aus = Join-Path $Temp "$n.out"
    $feh = Join-Path $Temp "$n.err"
    $text = @($argumente | ForEach-Object { if ($_ -eq '' -or $_ -match '[\s"]') { '"' + ($_ -replace '"', '\"') + '"' } else { $_ } }) -join ' '
    $sp = @{ FilePath = $exe; WorkingDirectory = $ordner; NoNewWindow = $true; PassThru = $true; RedirectStandardOutput = $aus; RedirectStandardError = $feh }
    if ($text) { $sp.ArgumentList = $text }
    if ($umgebung.Count -gt 0) { $sp.Environment = $umgebung }
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process @sp
    $null = $p.Handle
    if ($p.WaitForExit($frist * 1000)) { $p.WaitForExit(); $code = $p.ExitCode }
    else { try { $p.Kill($true) } catch { }; $null = $p.WaitForExit(10000); $code = "Frist $frist s ueberschritten" }
    $uhr.Stop()
    $o = Lies-Ausgabe $aus
    $e = Lies-Ausgabe $feh
    foreach ($f in @($aus, $feh)) { try { if (Test-Path -LiteralPath $f) { [IO.File]::Delete($f) } } catch { } }
    $zeige = if ($exe -eq $Py) { 'py' } elseif ($exe -eq $Cargo) { 'cargo' } elseif ($exe -eq $Pwsh) { 'pwsh' } else { Rel $exe }
    $umg = if ($umgebung.Count -gt 0) { '[' + (@($umgebung.Keys | Sort-Object | ForEach-Object { if ($_ -eq 'NAKAMA_NICHT_GELAUFEN') { "$_=<frischer Meldeordner>" } else { "$_=$($umgebung[$_])" } }) -join ', ') + '] ' } else { '' }
    return [pscustomobject]@{ Aufruf = ($umg + $zeige + $(if ($text) { ' ' + $text } else { '' })); Code = $code; Aus = $o; Fehler = $e; Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 1) }
}

# ---------------------------------------------------------------- Beine
function Lauf-Cpp([string]$bein) { @{ Exe = $Bin[$bein]; Args = @(); Art = $(if ($bein -eq 'B22') { 'fp' } else { 'cpp' }); Name = $bein } }
function Lauf-A36([string]$nur) { if ($nur) { @{ Exe = $Py; Args = @('-3.13', 'tools/eq-copilot/pruefe_beweisrunner.py', '--selbsttest', '--nur', $nur); Art = 'a36'; Name = "A36 --nur $nur" } } else { @{ Exe = $Py; Args = @('-3.13', 'tools/eq-copilot/pruefe_beweisrunner.py', '--selbsttest'); Art = 'a36'; Name = 'A36' } } }
function Lauf-A12 { @{ Exe = $Py; Args = @('-3.13', 'tools/eq-copilot/erzeuge_state_fixtures.py', '--pruefen'); Art = 'a12'; Name = 'A12' } }
function Lauf-Cargo([string[]]$args2, [string]$name, [hashtable]$umgebung = @{}) { @{ Exe = $Cargo; Args = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never') + $args2; Art = 'cargo'; Name = $name; Umgebung = $umgebung } }
function Lauf-Meldeweg([string[]]$args2, [string]$name, [string]$erwartet, [hashtable]$umgebung = @{}) { @{ Exe = $Cargo; Args = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never') + $args2; Art = 'meldeweg'; Name = $name; Erwartet = $erwartet; Umgebung = $umgebung } }

function Werte([string]$art, [string]$text) {
    $ok = New-Object System.Collections.Generic.List[string]
    $rot = New-Object System.Collections.Generic.List[string]
    $summe = New-Object System.Collections.Generic.List[string]
    $zeilen = @($text -split "`r?`n" | Where-Object { $_.Trim() })
    foreach ($z in $zeilen) {
        $z = $z.TrimEnd()
        switch ($art) {
            'cpp' {
                if ($z -match '^\s{2}ok\s') { $ok.Add($z) }
                elseif ($z -match '^\s{2}FEHLER\s') { $rot.Add($z) }
                elseif ($z -match '^(STATE-MIGRATION-TEST|ALLE PRUEFUNGEN|FEHLER —|[A-Z-]+-TEST) ') { $summe.Add($z) }
            }
            'fp' {
                if ($z -match '^\[ok\] ') { $ok.Add($z) }
                elseif ($z -match '^\[ROT\] ') { $rot.Add($z) }
                elseif ($z -match '^\d+ bestanden, \d+ gescheitert') { $summe.Add($z) }
            }
            'cargo' {
                if ($z -match '^test \S+ \.\.\. ok$') { $ok.Add($z) }
                elseif ($z -match '^test \S+ \.\.\. FAILED$') { $rot.Add($z) }
                elseif ($z -match "^thread '.+' .*panicked at") { }
                elseif ($z -match '^test result:') { $summe.Add($z) }
            }
            'a36' {
                # Die Pruefungszeilen eines Falls ohne ihr Kennwort, damit die Erwartung am Text haengt.
                if ($z -match '^\s{6}ok\s{3}') { $ok.Add(($z -replace '^\s{6}ok\s{3}', '')) }
                elseif ($z -match '^\s{6}ROT\s{2}') { $rot.Add(($z -replace '^\s{6}ROT\s{2}', '')) }
                elseif ($z -match '^Selbsttest:') { $summe.Add($z) }
            }
            'a12' {
                if ($z -match '^(FEHLT|ABWEICHUNG|VERWAIST): ') { $rot.Add($z) }
                elseif ($z -match '^STATE-FIXTURES ') { $summe.Add($z) }
            }
            'zaehlung' {
                if ($z -match 'widerspricht der Zaehlung: JA') { $rot.Add($z) }
                elseif ($z -match 'widerspricht der Zaehlung: NEIN') { $ok.Add($z) }
                elseif ($z -match '^\s+Summe\s|^Ohne eingefrorenes Fixture|^Karte \(') { $summe.Add($z.Trim()) }
            }
        }
    }
    if ($art -eq 'cargo') {
        # Die Panikmeldung steht unter "---- <test> stdout ----" in der Zeile nach "panicked at".
        $alle = @($text -split "`r?`n")
        for ($i = 0; $i -lt $alle.Count - 1; $i++) {
            if ($alle[$i] -match "^thread '(?<t>[^']+)' .*panicked at") { $rot.Add(("  Panik in {0}: {1}" -f $Matches['t'], $alle[$i + 1].Trim())) }
        }
    }
    return [pscustomobject]@{ Ok = $ok; Rot = $rot; Summe = $summe }
}

function Fahre-Lauf($lauf) {
    $umgebung = if ($lauf.Umgebung) { $lauf.Umgebung } else { @{} }
    if ($lauf.Art -eq 'meldeweg') {
        # Das Verfahren des Runners fuer A4 und A4-SI (tools/beweise.ps1): frischer Meldeordner nur in der Umgebung
        # des Kindprozesses, danach pruefe_beweisrunner.py --nicht-gelaufen; Beinbilanz wie im Runner.
        $ordner = Join-Path $Temp ('meldeweg-' + [guid]::NewGuid().ToString('N'))
        [void](New-Item -ItemType Directory -Path $ordner)
        $u = @{} + $umgebung
        $u['NAKAMA_NICHT_GELAUFEN'] = $ordner
        try {
            $r = Fahre $lauf.Exe $lauf.Args $Repo 1800 $u
            $k = Fahre $Py @('-3.13', 'tools/eq-copilot/pruefe_beweisrunner.py', '--nicht-gelaufen', $ordner) $Repo 120
        } finally {
            try { [IO.Directory]::Delete($ordner, $true) } catch { }
        }
        $w = Werte 'cargo' ($r.Aus + "`n" + $r.Fehler)
        $symbol = if ("$($r.Code)" -ne '0') { '[ROT]' } elseif ("$($k.Code)" -eq '3') { '[NOT RUN]' } elseif ("$($k.Code)" -eq '0') { '[OK]' } else { '[ROT]' }
        $marken = @(($k.Aus -split "`r?`n") | Where-Object { $_ -match '^\s+NOT RUN ' } | ForEach-Object { $_.Trim() })
        $bilanz = "Beinbilanz $($lauf.Name): $symbol (erwartet $($lauf.Erwartet))" + $(if ($marken.Count) { ' - ' + ($marken -join '; ') } else { '' })
        if ($symbol -eq $lauf.Erwartet) { $w.Ok.Add($bilanz) } else { $w.Rot.Add($bilanz) }
        $w.Summe.Add("Klassifikation: $((@(($k.Aus -split "`r?`n") | Where-Object { $_ -match '^Meldeweg:' }) -join ' ')) (Werkzeug-Exit $($k.Code))")
        return [pscustomobject]@{ Aufruf = "$($r.Aufruf); danach py -3.13 tools/eq-copilot/pruefe_beweisrunner.py --nicht-gelaufen <Meldeordner>"; Code = $r.Code; Ok = $w.Ok; Rot = $w.Rot; Summe = $w.Summe; Sekunden = $r.Sekunden }
    }
    $r = Fahre $lauf.Exe $lauf.Args $Repo 1800 $umgebung
    $w = Werte $lauf.Art ($r.Aus + "`n" + $r.Fehler)
    if ($lauf.ErwarteScheitern) {
        # Die Zusage verlangt, dass dieser Lauf scheitert (M-68): sein Scheitern ist die erwartete Pruefung, ein
        # Exit 0 der Rotfall.
        $w.Ok.AddRange($w.Rot)
        $w.Rot.Clear()
        $zeile = "$($lauf.Name): Exit $($r.Code) (erwartet: scheitert)"
        if ("$($r.Code)" -eq '0') { $w.Rot.Add("$zeile - besteht still") } else { $w.Ok.Add("$zeile - scheitert") }
    }
    return [pscustomobject]@{ Aufruf = $r.Aufruf; Code = $r.Code; Ok = $w.Ok; Rot = $w.Rot; Summe = $w.Summe; Sekunden = $r.Sekunden }
}

function Baue([string[]]$ziele, [string[]]$bezug, $zeilen) {
    # Nur das Ziel des Beins und danach der Zeitstempel des Binaries gegen jede mutierte Quelle (NAK-230).
    $gut = $true
    foreach ($z in $ziele) {
        $r = Fahre $script:Cmake @('--build', 'eq-copilot/build', '--config', 'Release', '--target', $z) $Repo 3600
        $zeilen.Add("Aufruf: cmake --build eq-copilot/build --config Release --target $z")
        $zeilen.Add("Exitcode: $($r.Code) ($($r.Sekunden) s)")
        foreach ($f in @(($r.Aus + "`n" + $r.Fehler) -split "`r?`n" | Where-Object { $_ -match '\berror\b|: Fehler|fatal' } | Select-Object -First 12)) { $zeilen.Add("  " + $f.Trim()) }
        $key = @($Ziel.Keys | Where-Object { $Ziel[$_] -eq $z })[0]
        if ($key) {
            $quellen = if ($bezug -and $bezug.Count) { @($bezug) } else { @($EtappenQuellen[$z] | ForEach-Object { Join-Path $Repo $_ }) }
            $binZeit = (Get-Item -LiteralPath $Bin[$key]).LastWriteTime
            foreach ($q in $quellen) {
                $qZeit = (Get-Item -LiteralPath $q).LastWriteTime
                $juenger = $binZeit -gt $qZeit
                $zeilen.Add(("Binary {0}: {1:yyyy-MM-dd HH:mm:ss.fff}, Quelle {2}: {3:yyyy-MM-dd HH:mm:ss.fff} - Binary juenger: {4}" -f (Rel $Bin[$key]), $binZeit, (Rel $q), $qZeit, $(if ($juenger) { 'ja' } else { 'NEIN' })))
                if (-not $juenger) { $gut = $false }
            }
        }
        if ("$($r.Code)" -ne '0') { $gut = $false }
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

function Git-Sauber([string]$datei) {
    & git -C $Repo diff --quiet -- $datei
    return ($LASTEXITCODE -eq 0)
}

function Basis-Bytes([string]$datei) {
    # Der Stand der Datei am Basis-SHA, byteweise (git show in eine Datei, keine Umkodierung).
    $ziel = Join-Path $Temp ('basis-' + [guid]::NewGuid().ToString('N'))
    $p = Start-Process -FilePath 'git' -ArgumentList '-C', $Repo, 'show', "${Basis}:$datei" -NoNewWindow -PassThru -Wait -RedirectStandardOutput $ziel
    if ($p.ExitCode -ne 0) { throw "git show ${Basis}:$datei scheitert" }
    $b = [IO.File]::ReadAllBytes($ziel)
    [IO.File]::Delete($ziel)
    return , $b
}

# ---------------------------------------------------------------- ein Rotbeweis
function Rotbeweis($s) {
    $dateien = @()
    foreach ($eintrag in $s.Dateien) {
        $pfad = Join-Path $Repo $eintrag.Datei
        $d = Lies-Datei $pfad
        $stellen = if ($eintrag.Ersetzungen) { Finde-Stellen $d $eintrag.Ersetzungen } else { @() }
        $dateien += [pscustomobject]@{ Datei = $eintrag.Datei; Pfad = $pfad; D = $d; Stellen = $stellen; Basisinhalt = [bool]$eintrag.Basisinhalt; Nur = [bool]$eintrag.NurSichern }
    }
    if ($Trocken) {
        foreach ($f in $dateien) { Write-Host ("[trocken] {0} {1}: {2}:{3} ok" -f $s.Zeile, $s.Teil, $f.Datei, $(if ($f.Basisinhalt) { 'Stand am Basis-SHA' } elseif ($f.Nur) { 'nur gesichert' } else { Stellen-Text $f.Stellen })) }
        return
    }
    foreach ($f in $dateien) { if (-not (Git-Sauber $f.Datei)) { throw "$($f.Datei) weicht von HEAD ab - kein Rotbeweis auf ungeprueftem Stand" } }
    $kopf = (& git -C $Repo rev-parse --short=8 HEAD).Trim()
    foreach ($f in $dateien) { $f | Add-Member -NotePropertyName ShaVorher -NotePropertyValue (Sha $f.Pfad) }
    $z = New-Object System.Collections.Generic.List[string]
    $z.Add("NAK-309 Etappe 4 - $(if ($s.Gegenprobe) { 'Gegenprobe' } else { 'Rotbeweis' }) $($s.Zeile) $($s.Teil)")
    $z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}" -f (Get-Date), $kopf))
    $z.Add("Zusage: $($s.Zusage)")
    $z.Add("Bein:   $($s.Bein)")
    foreach ($f in $dateien) {
        $wo = if ($f.Basisinhalt) { "(ganze Datei durch den Stand am Basis-SHA $($Basis.Substring(0, 8)) ersetzt)" } elseif ($f.Nur) { '(nur gesichert, der Vorlauf schreibt sie)' } else { ':' + (Stellen-Text $f.Stellen) }
        $z.Add("Datei:  $($f.Datei)$wo")
        $z.Add("SHA-256 vorher: $($f.ShaVorher) (Datei gleich HEAD: ja)")
    }
    $z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe4-rot.ps1 (SHA-256 $WerkzeugSha)")
    if ($s.OhneBasislauf) { $z.Add(''); $z.Add("Lauf gegen den Basisstand: $($s.OhneBasislauf)") }
    $z.Add('')
    $z.Add('--- MUTIERTE STELLE (die Zeile, die die Zusage traegt) ---')
    if ($dateien.Count -eq 0) { $z.Add("keine Quellmutation: $($s.OhneMutation)") }
    foreach ($f in $dateien) {
        if ($f.Basisinhalt) { $z.Add("$($f.Datei): ganze Datei = git show $($Basis.Substring(0, 8)):$($f.Datei)"); continue }
        if ($f.Nur) { continue }
        foreach ($st in $f.Stellen) {
            $z.Add("$($f.Datei), vorher (Zeile $($st.Index + 1)):")
            foreach ($l in $st.Alt) { $z.Add($l) }
            $z.Add('nachher:')
            foreach ($l in $st.Neu) { $z.Add($l) }
        }
    }

    $laeufe = @()
    $bauGut = $true
    try {
        foreach ($f in $dateien) {
            if ($f.Nur) { continue }
            $neu = if ($f.Basisinhalt) { Basis-Bytes $f.Datei } else { Mutierte-Bytes $f.D $f.Stellen }
            [IO.File]::WriteAllBytes($f.Pfad, $neu)
            (Get-Item -LiteralPath $f.Pfad).LastWriteTime = Get-Date
        }
        $z.Add('')
        $z.Add('--- BAU UNTER MUTATION ---')
        if ($s.Bau) { $bauGut = Baue $s.Bau @($dateien | Where-Object { -not $_.Nur } | ForEach-Object { $_.Pfad }) $z }
        else { $z.Add("kein Bau: $(if ($s.BauGrund) { $s.BauGrund } else { 'Rust baut cargo test selbst; Python und PowerShell lesen die Quelle beim Lauf' })") }
        if ($bauGut -and $s.Vorlauf) {
            $v = Fahre $s.Vorlauf.Exe $s.Vorlauf.Args $Repo 600
            $z.Add("Vorlauf: $($v.Aufruf) - Exit $($v.Code)")
            foreach ($l in @(($v.Aus + "`n" + $v.Fehler) -split "`r?`n" | Where-Object { $_.Trim() } | Select-Object -Last 3)) { $z.Add("  $l") }
        }
        if ($bauGut) { foreach ($l in $s.Laeufe) { $laeufe += Fahre-Lauf $l } }
    } finally {
        foreach ($f in $dateien) {
            [IO.File]::WriteAllBytes($f.Pfad, $f.D.Bytes)
            (Get-Item -LiteralPath $f.Pfad).LastWriteTime = Get-Date
        }
    }
    $sauber = $true
    $shaGleich = $true
    foreach ($f in $dateien) {
        $f | Add-Member -NotePropertyName ShaNachher -NotePropertyValue (Sha $f.Pfad)
        if ($f.ShaNachher -ne $f.ShaVorher) { $shaGleich = $false }
        if (-not (Git-Sauber $f.Datei)) { $sauber = $false }
    }

    $z.Add('')
    $z.Add('--- LAUF UNTER MUTATION ---')
    if (-not $bauGut) { $z.Add('Bau unter Mutation gescheitert - kein Lauf, kein Beleg.') }
    foreach ($l in $laeufe) {
        $z.Add("Aufruf: $($l.Aufruf)")
        $z.Add("Exitcode: $($l.Code) ($($l.Sekunden) s)")
        foreach ($r in $l.Rot) { $z.Add("  $r") }
        foreach ($r in $l.Summe) { $z.Add("  $r") }
        $z.Add('')
    }
    $alleRot = New-Object System.Collections.Generic.List[string]
    foreach ($l in $laeufe) { foreach ($r in $l.Rot) { if (-not $alleRot.Contains($r)) { $alleRot.Add($r) } } }
    $treffer = @($s.Erwartet | ForEach-Object { $m = $_.Muster; [pscustomobject]@{ Text = $_.Text; Gefallen = (@($alleRot | Where-Object { $_ -match $m }).Count -gt 0) } })
    if ($s.Gegenprobe) {
        # Eine Gegenprobe erwartet GRUEN (Behauptung, die heute nichts misst, oder ein Stand, der die Luecke zeigt).
        $gruen = $bauGut -and $alleRot.Count -eq 0 -and @($laeufe | Where-Object { "$($_.Code)" -ne '0' }).Count -eq 0
        $z.Add('Gefallene Pruefungen:')
        if ($alleRot.Count -eq 0) { $z.Add('  keine') }
        foreach ($r in $alleRot) { $z.Add("  $r") }
        $z.Add('')
        $z.Add("Gegenprobe wie erwartet (der Lauf bleibt gruen): $(if ($gruen) { 'JA' } else { 'NEIN' })")
        $ja = $gruen
    }
    else {
        $z.Add('Gefallene Pruefungen:')
        if ($alleRot.Count -eq 0) { $z.Add('  keine') }
        foreach ($r in $alleRot) { $z.Add("  $r") }
        $z.Add('')
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
    }
    $z.Add('')
    $gruenListe = if ($laeufe.Count -gt 0) { @($laeufe[-1].Ok) } else { @() }
    $z.Add("Unter derselben Mutation weiter gruen (Beleg gegen den Nebeneffekt): $($gruenListe.Count)")
    foreach ($g in $gruenListe) { $z.Add("  $g") }
    if ($s.Hinweis) { $z.Add(''); $z.Add("Hinweis: $($s.Hinweis)") }

    $z.Add('')
    $z.Add('--- LAUF NACH RUECKNAHME ---')
    $z.Add("Quellen bytegleich zurueckgespielt: $(if ($shaGleich) { 'ja' } else { 'NEIN' }) ($(@($dateien | ForEach-Object { "$($_.Datei) SHA-256 $($_.ShaNachher)" }) -join '; ')); LastWriteTime auf jetzt gesetzt; git diff leer: $(if ($sauber) { 'ja' } else { 'NEIN' })")
    $nachGut = $shaGleich -and $sauber
    if ($s.Bau) { if (-not (Baue $s.Bau @($dateien | Where-Object { -not $_.Nur } | ForEach-Object { $_.Pfad }) $z)) { $nachGut = $false } }
    $nachLaeufe = if ($s.NachLaeufe) { $s.NachLaeufe } else { $s.Laeufe }
    foreach ($l in $nachLaeufe) {
        $n = Fahre-Lauf $l
        $z.Add("Aufruf: $($n.Aufruf)")
        $z.Add("Exitcode: $($n.Code) ($($n.Sekunden) s)")
        foreach ($r in $n.Rot) { $z.Add("  $r") }
        foreach ($r in $n.Summe) { $z.Add("  $r") }
        if (-not $l.ErwarteScheitern -and "$($n.Code)" -ne '0') { $nachGut = $false }
        if ($n.Rot.Count -gt 0) { $nachGut = $false }
    }
    $z.Add('')
    $z.Add("Bein nach Ruecknahme gruen (die Zusage haelt): $(if ($nachGut) { 'ja' } else { 'NEIN' })")

    $roh = Join-Path $RohOrdner ("NAK-309-rot-{0}.txt" -f $s.Zeile)
    $text = [string]::Join("`n", $z) + "`n"
    if ($script:Geschrieben.Contains($roh)) { $text = "`n==============================================================================`n" + $text; [IO.File]::AppendAllText($roh, $text, $Utf8) }
    else { [IO.File]::WriteAllText($roh, $text, $Utf8); [void]$script:Geschrieben.Add($roh) }
    $wort = if ($s.Gegenprobe) { 'Gegenprobe wie erwartet' } else { 'faellt an der Zusagezeile' }
    Write-Host ("{0} {1}: {2} {3}, nach Ruecknahme gruen {4}" -f $s.Zeile, $s.Teil, $wort, $(if ($ja) { 'JA' } else { 'NEIN' }), $(if ($nachGut) { 'ja' } else { 'NEIN' }))
}

# ---------------------------------------------------------------- Mutationen
$S = New-Object System.Collections.Generic.List[object]
function Spec([hashtable]$h) { $S.Add([pscustomobject]$h) }
function E([string]$muster, [string]$text) { [pscustomobject]@{ Muster = $muster; Text = $text } }

$B2Bein = 'B2 EqCopStateMigrationTest (eq-copilot/plugin/tests/StateMigrationTestMain.cpp)'
$B10Bein = 'B10 EqCopIpcTest (eq-copilot/plugin/tests/IpcTestMain.cpp)'
$B22Bein = 'B22 EqCopSonde013FingerprintGoldenTest (eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp)'
$A36Bein = 'A36 (tools/eq-copilot/pruefe_beweisrunner.py --selbsttest); gezielt der Fall ueber --nur, dazu das ganze Bein'
$CppOhneBasis = 'ohne Umbau nicht moeglich - der neue Fall steht im Testbinary des Etappenstands; das Binary des Basis-SHA kennt ihn nicht.'
$PyOhneBasis = 'ohne Umbau nicht moeglich - Selbsttest und Code stehen in derselben Datei.'
$RustOhneBasis = 'ohne Umbau nicht moeglich - Test und Hilfe stehen im Testcrate des Etappenstands.'
$G7Kennzeile = 'Abschnittsurteil von G7 (StateMigrationTestMain.cpp, Abschnitt::schliesse): die Kennzeile faellt mit jedem Einzelfehler ihres Abschnitts - erwartet'

# --- M-57 bis M-60: Writer-Golden main-binding-v1 (B2, A12) ---
Spec @{ Zeile = 'M-57'; Teil = 'Leser weist einen leeren Wert zu'; Bein = $B2Bein
    Zusage = 'Das eingefrorene Byte-Bild main-binding-v1.bin laedt mit geladen, nicht read-only, und traegt die Bindung unveraendert.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/state/NakamaState.cpp'; Ersetzungen = @(@{
Alt = @'
    c.projectBindingId = binding.toString();
'@
Neu = @'
    c.projectBindingId = juce::String(); // MUTATION: der Leser verliert die Bindung
'@ }) })
    Bau = @('EqCopStateMigrationTest'); Laeufe = @((Lauf-Cpp 'B2'))
    OhneBasislauf = 'entfaellt - erlaubtes Verhalten gemessen (der Leser liest das Feld heute, gemessen war es nur am frischen Roundtrip).'
    Erwartet = @((E 'FEHLER\s+M-57 main-binding-v1' 'M-57 main-binding-v1: das eingefrorene Byte-Bild laedt ... und traegt die Bindung unveraendert'))
    Mit = @{ 'M-58 main-binding-v1: Speichern nach dem Laden' = 'der Rueckweg laedt ueber denselben Leser; ohne Bindung schreibt der Writer sie nicht zurueck - erwartet, die Zusage von M-58 (Writer gegen die Platte) bleibt gruen'; '== Migration: 4 Rollen bytegleich zum Golden, idempotent' = $G7Kennzeile } }

Spec @{ Zeile = 'M-58'; Teil = 'Writer entfernt die Bindung bedingungslos'; Bein = $B2Bein
    Zusage = 'Der heutige Writer erzeugt genau die eingefrorenen Bytes, und Speichern nach dem Laden des Goldens ist bytegleich.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/state/NakamaState.cpp'; Ersetzungen = @(@{
Alt = @'
    if (z.common.projectBindingId.isNotEmpty()) common.setProperty (kBinding, z.common.projectBindingId, nullptr);
    else                                        common.removeProperty (kBinding, nullptr);
'@
Neu = @'
    common.removeProperty (kBinding, nullptr); // MUTATION: bedingungslos entfernt
'@ }) })
    Bau = @('EqCopStateMigrationTest'); Laeufe = @((Lauf-Cpp 'B2'))
    OhneBasislauf = 'entfaellt - erlaubtes Verhalten gemessen.'
    Erwartet = @((E 'FEHLER\s+M-58 main-binding-v1: der heutige Writer' 'M-58 main-binding-v1: der heutige Writer erzeugt genau die eingefrorenen Bytes'), (E 'FEHLER\s+M-58 main-binding-v1: Speichern nach dem Laden' 'M-58 main-binding-v1: Speichern nach dem Laden des Goldens ist bytegleich'))
    Mit = @{ '== Migration: 4 Rollen bytegleich zum Golden, idempotent' = $G7Kennzeile }
    Hinweis = 'M-57 liest die Platte und bleibt unter dieser Mutation gruen (siehe Liste unten): Leser und Schreiber stehen nicht auf demselben Writer.' }

Spec @{ Zeile = 'M-59'; Teil = '(1) Golden nicht in der Goldenliste'; Bein = 'A12 (py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen)'
    Zusage = 'Das neue Golden steht in der Goldenliste und im MANIFEST, bytegleich zur Neuerzeugung, ohne verwaiste Datei.'
    Dateien = @(@{ Datei = 'tools/eq-copilot/erzeuge_state_fixtures.py'; Ersetzungen = @(@{
Alt = @'
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1", "main-binding-v1"):
'@
Neu = @'
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1"):  # MUTATION: main-binding-v1 nicht eingetragen
'@ }) })
    Laeufe = @((Lauf-A12))
    OhneBasislauf = 'entfaellt - Regressionswache der Registrierung; die Waisenpruefung ueber schema2/ kommt aus dieser Etappe (Teil 2 zeigt den alten Umfang).'
    Erwartet = @((E '^VERWAIST: schema2/main-binding-v1\.bin' 'VERWAIST: schema2/main-binding-v1.bin'))
    Mit = @{ '^ABWEICHUNG: MANIFEST\.json' = 'das committete MANIFEST fuehrt das Golden, die Neuerzeugung ohne Listeneintrag nicht - zweite Schicht derselben Zusage (bytegleich zur Neuerzeugung)' } }

Spec @{ Zeile = 'M-59'; Teil = '(2) Gegenprobe: alter Waisenumfang ohne schema2/'; Bein = 'A12 (py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen)'; Gegenprobe = $true
    Zusage = 'Gegenprobe zur Etappenaenderung: mit dem Waisenumfang vor Etappe 4 (jcs, dto, preset) und einem MANIFEST, das der Erzeuger ohne Listeneintrag neu geschrieben hat, bleibt eine verwaiste Datei unter schema2/ unbemerkt - deshalb gehoert schema2/ in die Waisenpruefung.'
    Dateien = @(@{ Datei = 'tools/eq-copilot/erzeuge_state_fixtures.py'; Ersetzungen = @(@{
Alt = @'
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1", "main-binding-v1"):
'@
Neu = @'
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1"):  # GEGENPROBE: main-binding-v1 nicht eingetragen
'@ }, @{
Alt = @'
        for ordner in ("jcs", "dto", "preset", "schema2"):
'@
Neu = @'
        for ordner in ("jcs", "dto", "preset"):  # GEGENPROBE: Waisenumfang vor Etappe 4
'@ }) }, @{ Datei = 'eq-copilot/fixtures/state/MANIFEST.json'; NurSichern = $true })
    Vorlauf = @{ Exe = $Py; Args = @('-3.13', 'tools/eq-copilot/erzeuge_state_fixtures.py') }
    Laeufe = @((Lauf-A12))
    OhneBasislauf = 'entfaellt - Gegenprobe des alten Waisenumfangs.' }

Spec @{ Zeile = 'M-60'; Teil = 'Migration erfindet eine Bindung'; Bein = $B2Bein
    Zusage = 'Die Migration erfindet keine Bindung (project_binding_id leer).'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/state/NakamaState.cpp'; Ersetzungen = @(@{
Alt = @'
    // project_binding_id wird NICHT erfunden (§32.2).
'@
Neu = @'
    c.projectBindingId = "ffffffffffffffffffffffffffffffff"; // MUTATION: die Migration erfindet eine Bindung
'@ }) })
    Bau = @('EqCopStateMigrationTest'); Laeufe = @((Lauf-Cpp 'B2'))
    OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E "FEHLER\s+'sensor': project_binding_id NICHT erfunden" "'sensor': project_binding_id NICHT erfunden (StateMigrationTestMain.cpp, G7)"), (E "FEHLER\s+'hub': project_binding_id NICHT erfunden" "'hub': project_binding_id NICHT erfunden"))
    Mit = @{ "project_binding_id NICHT erfunden" = 'dieselbe Pruefung fuer die uebrigen Rollen'; "Golden '.+' weicht ab|4 Rollen bytegleich zum Schema-2-Golden" = 'die Migration schreibt die erfundene Bindung, die eingefrorenen Migrationsgoldens tragen keine - dieselbe Zusage an den Bytes'; '== Migration: 4 Rollen bytegleich zum Golden, idempotent' = $G7Kennzeile } }

# --- M-61: Feldzaehlung ---
Spec @{ Zeile = 'M-61'; Teil = 'Zaehlung ohne das Golden'; Bein = 'Zaehlwerkzeug docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py (Rohdatei NAK-309-etappe4-feldzaehlung.txt)'
    Zusage = 'Die Zahl ist gemessen: Karte und KONZEPT nennen genau den Wert der Zaehlung ueber alle eingefrorenen Byte-Bilder.'
    Dateien = @(); OhneMutation = 'das Golden main-binding-v1.bin wird in der Zaehlung weggelassen (--ohne) - der Stand vor dem neuen Golden'
    Laeufe = @(@{ Exe = $Py; Args = @('-3.13', 'docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py', '--ohne', 'schema2/main-binding-v1.bin'); Art = 'zaehlung'; Name = 'Zaehlung ohne Golden' })
    NachLaeufe = @(@{ Exe = $Py; Args = @('-3.13', 'docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py'); Art = 'zaehlung'; Name = 'Zaehlung' })
    OhneBasislauf = 'der Basisstand ist der Lauf "vorher" in NAK-309-etappe4-feldzaehlung.txt (132, Karte 133).'
    Erwartet = @((E 'widerspricht der Zaehlung: JA' 'Karte oder KONZEPT widerspricht der Zaehlung: JA (132 gegen 133)')) }

# --- M-62 bis M-65: Welcome-Minorgrenze (B10) ---
Spec @{ Zeile = 'M-62'; Teil = 'Control-Welcome verlangt schema_minor < kJsonSchemaMinor'; Bein = $B10Bein
    Zusage = 'Ein welcome mit Envelope-schema_minor 5 (= kJsonSchemaMinor) verbindet.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp'; Ersetzungen = @(@{
Alt = @'
            welcomeKam = true;
'@
Neu = @'
            if (e.kopf.schemaMinor >= kJsonSchemaMinor) break; // MUTATION: Welcome verlangt schema_minor < kJsonSchemaMinor
            welcomeKam = true;
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = $CppOhneBasis
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_control \(M-62\)' 'welcome_minor_grenze_control (M-62)'))
    Mit = @{ 'welcome_minor_grenze_control \(M-63 a\)' = 'Minor 6 ist ebenfalls >= 5 - dieselbe Mutation trifft den Grenzfall darueber'; 'welcome_minor_grenze_control \(M-63 b\)' = 'ohne Verbindung kommt kein Frame mit Minor 6 an, also keine Abweisung' } }

Spec @{ Zeile = 'M-63'; Teil = '(a) Control-Welcome prueft den Minor (> kJsonSchemaMinor)'; Bein = $B10Bein
    Zusage = '(a) heute erlaubt: ein welcome mit schema_minor 6 verbindet.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp'; Ersetzungen = @(@{
Alt = @'
            welcomeKam = true;
'@
Neu = @'
            if (e.kopf.schemaMinor > kJsonSchemaMinor) break; // MUTATION: Minorpruefung im Welcome
            welcomeKam = true;
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = $CppOhneBasis
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_control \(M-63 a\)' 'welcome_minor_grenze_control (M-63 a)'))
    Mit = @{ 'welcome_minor_grenze_control \(M-63 b\)' = 'ohne Verbindung kommt kein Frame mit Minor 6 an, also keine Abweisung' }
    Hinweis = 'M-62 (Minor 5) bleibt unter dieser Mutation gruen - die Grenze liegt genau zwischen 5 und 6.' }

Spec @{ Zeile = 'M-63'; Teil = '(b) Eingangspruefung nach dem Welcome entfernt'; Bein = $B10Bein
    Zusage = '(b) der erste Frame mit Minor 6 schliesst: letzterFehler "Envelope schema_minor ist neuer als der JSON-Leser - wird geschlossen", envelopeAbweisungen +1.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp'; Ersetzungen = @(@{
Alt = @'
            if (e.kopf.schemaMinor > kJsonSchemaMinor)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "Envelope schema_minor ist neuer als der JSON-Leser — wird geschlossen";
                ++zustand.envelopeAbweisungen;
                return false;
            }
'@
Neu = @'
            // MUTATION: Pruefung des Envelope-Minors im Eingang entfernt
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = 'entfaellt - Regressionswache der bestehenden Pruefung.'
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_control \(M-63 b\)' 'welcome_minor_grenze_control (M-63 b)'))
    Mit = @{ 'hoeherer unbekannter JSON-Minor schliesst' = 'G1a misst dieselbe Eingangspruefung an einer Antwort mit Minor 6 - dieselbe Zeile' } }

Spec @{ Zeile = 'M-64'; Teil = 'Telemetrie-Welcome verlangt schema_minor < kJsonSchemaMinor'; Bein = $B10Bein
    Zusage = 'Ein Telemetrie-welcome mit schema_minor 5 koppelt.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/TelemetryClient.cpp'; Ersetzungen = @(@{
Alt = @'
            welcomeKam = true;
'@
Neu = @'
            if (e.kopf.schemaMinor >= kJsonSchemaMinor) break; // MUTATION: Welcome verlangt schema_minor < kJsonSchemaMinor
            welcomeKam = true;
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = $CppOhneBasis
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_telemetrie \(M-64\)' 'welcome_minor_grenze_telemetrie (M-64)'))
    Mit = @{ 'welcome_minor_grenze_telemetrie \(M-65\)' = 'Minor 6 ist ebenfalls >= 5, ohne Kopplung kommt kein P2-Frame an' } }

Spec @{ Zeile = 'M-65'; Teil = '(1) beiFrame bekommt den gedeckelten Minor'; Bein = $B10Bein
    Zusage = 'Heute erlaubt: der P2-Frame mit schema_minor 3 erreicht beiFrame mit Minor 3 unveraendert.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/TelemetryClient.cpp'; Ersetzungen = @(@{
Alt = @'
            beiFrame (e.payload, e.payloadLaenge, e.kopf.schemaMinor);
'@
Neu = @'
            beiFrame (e.payload, e.payloadLaenge, std::min (e.kopf.schemaMinor, kFeatureBatchSchemaMinor)); // MUTATION: Minor gedeckelt
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = $CppOhneBasis
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_telemetrie \(M-65\): der P2-Frame danach' 'welcome_minor_grenze_telemetrie (M-65): der P2-Frame ... erreicht beiFrame mit schema_minor 3 unveraendert')) }

Spec @{ Zeile = 'M-65'; Teil = '(2) Telemetrie-Welcome prueft den Minor (> kJsonSchemaMinor)'; Bein = $B10Bein
    Zusage = 'Heute erlaubt: ein Telemetrie-welcome mit schema_minor 6 koppelt.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/ipc/TelemetryClient.cpp'; Ersetzungen = @(@{
Alt = @'
            welcomeKam = true;
'@
Neu = @'
            if (e.kopf.schemaMinor > kJsonSchemaMinor) break; // MUTATION: Minorpruefung im Welcome
            welcomeKam = true;
'@ }) })
    Bau = @('EqCopIpcTest'); Laeufe = @((Lauf-Cpp 'B10')); OhneBasislauf = $CppOhneBasis
    Erwartet = @((E 'FEHLER\s+welcome_minor_grenze_telemetrie \(M-65\): ein welcome mit schema_minor 6' 'welcome_minor_grenze_telemetrie (M-65): ein welcome mit schema_minor 6 koppelt'))
    Mit = @{ 'welcome_minor_grenze_telemetrie \(M-65\): der P2-Frame danach' = 'ohne Kopplung kommt kein P2-Frame an' }
    Hinweis = 'M-64 (Minor 5) bleibt unter dieser Mutation gruen.' }

# --- M-66: Briefkasten (A4, Testmodul) ---
$M66Lauf = Lauf-Cargo @('--lib', 'briefkasten::tests::nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal', '--', '--nocapture') 'M-66'
$BkLauf = Lauf-Cargo @('--lib', 'briefkasten::tests::') 'briefkasten::tests'
Spec @{ Zeile = 'M-66'; Teil = '(1) Stoppflag vor jedem Takt entfernt'; Bein = 'A4 (cargo test, Testmodul broker/src/briefkasten.rs); gezielt der Test, dazu das ganze Testmodul'
    Zusage = 'Nach der Join-Frist beendet der Thread den begonnenen Schreibschritt genau einmal (eine Antwortdatei, takte_fertig +1) und beginnt keinen neuen Takt.'
    Dateien = @(@{ Datei = 'broker/src/briefkasten.rs'; Ersetzungen = @(@{
Alt = @'
            // Das Stoppflag vor jedem Takt und nach jedem Wecken (M-45).
            if weck.stopp {
                break;
            }
'@
Neu = @'
            // MUTATION: Stoppflag vor dem Takt entfernt
'@ }) })
    Laeufe = @($M66Lauf, $BkLauf); OhneBasislauf = $RustOhneBasis
    Erwartet = @((E 'nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal \.\.\. FAILED' 'nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal'), (E 'Takte fertig nach dem Stopp statt genau des begonnenen' 'Meldung: <n> Takte fertig nach dem Stopp statt genau des begonnenen'))
    Mit = @{ 'nach_stopp_keine_lieferung|stopp_weckt_ohne_wartedurchlauf|stoppreihenfolge_briefkasten_zuerst' = 'M-45 (1) bis (3) aus NAK-286 messen dieselbe Stoppruefung: ohne sie endet der Thread im Stopp nie und beginnt nach dem Stopp weitere Takte' } }

Spec @{ Zeile = 'M-66'; Teil = '(2) begonnener Abschluss nach dem Stopp abgewuergt'; Bein = 'A4 (cargo test, Testmodul broker/src/briefkasten.rs); gezielt der Test, dazu das ganze Testmodul'
    Zusage = 'Der begonnene Schreibschritt endet nach der Freigabe genau einmal - mit genau einer Antwortdatei.'
    Dateien = @(@{ Datei = 'broker/src/briefkasten.rs'; Ersetzungen = @(@{
Alt = @'
        // (5) ohne Ersetzen in den Antwortnamen. (6) kein Aufraeumer.
        dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel)
'@
Neu = @'
        // (5) ohne Ersetzen in den Antwortnamen. (6) kein Aufraeumer.
        if sperre(&self.weck).stopp { return false; } // MUTATION: Abschluss nach dem Stopp abgewuergt
        dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel)
'@ }) })
    Laeufe = @($M66Lauf, $BkLauf); OhneBasislauf = $RustOhneBasis
    Erwartet = @((E 'nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal \.\.\. FAILED' 'nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal'), (E 'Antwortdatei\(en\) statt genau einer' 'Meldung: 0 Antwortdatei(en) statt genau einer'))
    Hinweis = 'Zweite Richtung aus der Quellvalidierung (Teil A, T3-04-04, Abschnitt 5): der erlaubte Abschluss darf nicht abgewuergt werden.' }

# --- M-67 bis M-69: NOT RUN (A4, A4-SI) ---
$Weist = @('--test', 'store_crash_matrix', 'store_weist_reparse_punkt_im_pfad_ab')
$Volumen = @('--test', 'store_crash_matrix', 'volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen', '--', '--ignored')
$Schalter = @{ NAKAMA_TEST_JUNCTION_VERWEIGERN = '1' }
Spec @{ Zeile = 'M-67'; Teil = '(1a) alter Skip ohne Marke (A4-Test)'; Bein = 'A4 im Verfahren des Runners (Meldeordner, --nicht-gelaufen) mit NAKAMA_TEST_JUNCTION_VERWEIGERN=1'
    Zusage = 'Die fehlende Voraussetzung endet als NOT RUN mit Grund: Marke im Meldeordner, Beinbilanz [NOT RUN], nie gruen.'
    Dateien = @(@{ Datei = 'broker/tests/store_crash_matrix.rs'; Ersetzungen = @(@{
Alt = @'
        nicht_gelaufen(
            "store_weist_reparse_punkt_im_pfad_ab",
            &format!("Reparse-Fall nicht gemessen: {grund}"),
        );
'@
Neu = @'
        eprintln!("mklink /J nicht verfuegbar; Reparse-Fall uebersprungen ({grund})"); // MUTATION: alter Skip ohne Marke
'@ }) })
    Laeufe = @((Lauf-Meldeweg $Weist 'A4 (store_weist_reparse_punkt_im_pfad_ab)' '[NOT RUN]' $Schalter))
    OhneBasislauf = 'heute rot laut Gegenprobe der Matrixzeile (store_crash_matrix.rs:3239-3244 am Basis-SHA: eprintln! und return, A4 ohne --nocapture); ohne Umbau nicht messbar, weil der Basisstand weder den Schalter noch den Meldeweg kennt.'
    Erwartet = @((E 'Beinbilanz A4 \(store_weist_reparse_punkt_im_pfad_ab\): \[OK\] \(erwartet \[NOT RUN\]\)' 'Beinbilanz [OK] statt [NOT RUN]')) }

Spec @{ Zeile = 'M-67'; Teil = '(1b) alter Skip ohne Marke (A4-SI-Test)'; Bein = 'A4-SI im Verfahren des Runners (Meldeordner, --nicht-gelaufen) mit NAKAMA_TEST_JUNCTION_VERWEIGERN=1'
    Zusage = 'Die fehlende Voraussetzung endet als NOT RUN mit Grund: Marke im Meldeordner, Beinbilanz [NOT RUN], nie gruen.'
    Dateien = @(@{ Datei = 'broker/tests/store_crash_matrix.rs'; Ersetzungen = @(@{
Alt = @'
        nicht_gelaufen(
            "volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen",
            &format!("TOCTOU-Fall nicht gemessen: {grund}"),
        );
'@
Neu = @'
        eprintln!("mklink /J nicht verfuegbar; TOCTOU-Fall uebersprungen ({grund})"); // MUTATION: alter Skip ohne Marke
'@ }) })
    Laeufe = @((Lauf-Meldeweg $Volumen 'A4-SI (volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen)' '[NOT RUN]' $Schalter))
    OhneBasislauf = 'heute rot laut Gegenprobe der Matrixzeile (store_crash_matrix.rs:3364-3367 am Basis-SHA); ohne Umbau nicht messbar.'
    Erwartet = @((E 'Beinbilanz A4-SI .+: \[OK\] \(erwartet \[NOT RUN\]\)' 'Beinbilanz [OK] statt [NOT RUN]')) }

Spec @{ Zeile = 'M-67'; Teil = '(2) Klassifikation zaehlt keine Marke'; Bein = $A36Bein
    Zusage = 'Eine Marke <Test>: <Grund> im Meldeordner ist NOT RUN (Werkzeug-Exit 3), nie leer.'
    Dateien = @(@{ Datei = 'tools/eq-copilot/pruefe_beweisrunner.py'; Ersetzungen = @(@{
Alt = @'
    if not marken:
'@
Neu = @'
    if True:  # MUTATION: Marken zaehlen nicht, der Meldeordner gilt als leer
'@ }) })
    Laeufe = @((Lauf-A36 'M-67'), (Lauf-A36 '')); OhneBasislauf = $PyOhneBasis
    Erwartet = @((E 'eine Marke <Test>: <Grund> ist NOT RUN' 'nicht_gelaufen_macht_unvollstaendig: eine Marke ist NOT RUN'))
    Mit = @{ 'zwei Marken desselben Tests|eine Marke eines anderen Tests' = 'dieselbe Zaehlung an weiteren Marken' } }

Spec @{ Zeile = 'M-68'; Teil = 'ohne Meldeweg stilles Ok'; Bein = 'A4 (cargo test --test store_crash_matrix)'
    Zusage = 'Ohne Meldeweg scheitert der Test mit "NOT RUN ohne Meldeweg: <Grund>" - nie Exit 0 ohne Marke.'
    Dateien = @(@{ Datei = 'broker/tests/store_crash_matrix.rs'; Ersetzungen = @(@{
Alt = @'
        return Err(format!("NOT RUN ohne Meldeweg: {grund}"));
'@
Neu = @'
        return Ok(PathBuf::new()); // MUTATION: ohne Meldeweg stilles Ok
'@ }) })
    Laeufe = @((Lauf-Cargo @('--test', 'store_crash_matrix', 'nicht_gelaufen_ohne_meldeweg_scheitert') 'M-68'),
               (@{ Exe = $Cargo; Args = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never') + $Weist; Art = 'cargo'; Name = 'store_weist_reparse_punkt_im_pfad_ab mit Schalter ohne Meldeweg'; Umgebung = $Schalter; ErwarteScheitern = $true }))
    OhneBasislauf = 'entfaellt - heute nicht messbar (die Funktion entsteht in dieser Etappe).'
    Erwartet = @((E 'nicht_gelaufen_ohne_meldeweg_scheitert \.\.\. FAILED' 'nicht_gelaufen_ohne_meldeweg_scheitert'), (E 'mit Schalter ohne Meldeweg: Exit 0 .*besteht still' 'store_weist_reparse_punkt_im_pfad_ab mit Schalter ohne Meldeweg besteht still'))
    Mit = @{ 'Panik in nicht_gelaufen_ohne_meldeweg_scheitert' = 'Meldung desselben Tests' } }

Spec @{ Zeile = 'M-69'; Teil = '(1) Marke trotz vorhandener Junction'; Bein = 'A4 im Verfahren des Runners (Meldeordner, --nicht-gelaufen), Schalter nicht gesetzt'
    Zusage = 'Die zwei Tests laufen wirklich: der Meldeordner bleibt leer, das Bein steht als [OK].'
    Dateien = @(@{ Datei = 'broker/tests/store_crash_matrix.rs'; Ersetzungen = @(@{
Alt = @'
    if let Err(grund) = junction_legen(&verweis, &echt) {
'@
Neu = @'
    if let Err(grund) = junction_legen(&verweis, &echt).and_then(|()| Err::<(), String>("Marke trotz vorhandener Junction".into())) { // MUTATION
'@ }) })
    Laeufe = @((Lauf-Meldeweg $Weist 'A4 (store_weist_reparse_punkt_im_pfad_ab)' '[OK]'))
    OhneBasislauf = 'entfaellt - heute nicht messbar (der Meldeordner entsteht in dieser Etappe).'
    Erwartet = @((E 'Beinbilanz A4 \(store_weist_reparse_punkt_im_pfad_ab\): \[NOT RUN\] \(erwartet \[OK\]\)' 'Beinbilanz [NOT RUN] statt [OK]')) }

Spec @{ Zeile = 'M-69'; Teil = '(2) NAK-121 G2-TOCTOU-002 wiederholt: Reparse-Riegel entfernt'; Bein = 'A4 im Verfahren des Runners, Schalter nicht gesetzt'
    Zusage = 'Die Rotbeweise der NAK-121-Zeilen laufen auf dem Stand der Etappe noch einmal und machen genau diese Tests rot (docs/beweise/roh/NAK-121-rot-G2-TOCTOU-002.txt).'
    Dateien = @(@{ Datei = 'broker/src/store/pfad.rs'; Ersetzungen = @(@{
Alt = @'
    let mut pruefling = Some(kandidat);
    while let Some(teil) = pruefling {
        if traegt_reparse_punkt(teil)? {
            return Err(StoreFehler::Pfad(format!(
                "Reparse-Punkt im Storepfad: {}",
                teil.display()
            )));
        }
        pruefling = teil.parent().filter(|eltern| !eltern.as_os_str().is_empty());
    }
'@
Neu = @'
    // MUTANTE: kein Reparse-Riegel - eine untergeschobene Junction
    // klassifiziert wieder den Vorfahren statt das geoeffnete Objekt.
'@ }) })
    Laeufe = @((Lauf-Meldeweg $Weist 'A4 (store_weist_reparse_punkt_im_pfad_ab)' '[OK]'))
    OhneBasislauf = 'entfaellt - Regressionswache (Wiederholung der NAK-121-Mutation).'
    Erwartet = @((E 'store_weist_reparse_punkt_im_pfad_ab \.\.\. FAILED' 'store_weist_reparse_punkt_im_pfad_ab (Exit 101)'))
    Mit = @{ 'Beinbilanz A4 .+: \[ROT\]' = 'der rote Test macht das Bein rot - ROT geht vor'; 'Panik in store_weist' = 'Meldung desselben Tests' } }

Spec @{ Zeile = 'M-69'; Teil = '(3) NAK-121 R2-3 wiederholt: zweites CreateFileW ueber den Namen'; Bein = 'A4-SI im Verfahren des Runners, Schalter nicht gesetzt'
    Zusage = 'Die Rotbeweise der NAK-121-Zeilen laufen auf dem Stand der Etappe noch einmal und machen genau diese Tests rot (docs/beweise/roh/NAK-121-rot-R2-3-r2.txt).'
    Dateien = @(@{ Datei = 'broker/src/store/pfad.rs'; Ersetzungen = @(@{
Alt = @'
    volume_am_handle(griff, "der geoeffneten SQLite-Hauptdatenbank")
'@
Neu = @'
    geoeffnete_db_volume(Path::new(conn.path().unwrap_or_default())) // MUTANTE: zweites CreateFileW ueber den Namen
'@ }) })
    Laeufe = @((Lauf-Meldeweg $Volumen 'A4-SI (volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen)' '[OK]'))
    OhneBasislauf = 'entfaellt - Regressionswache (Wiederholung der NAK-121-Mutation).'
    Erwartet = @((E 'volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen \.\.\. FAILED' 'volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen (Exit 101)'))
    Mit = @{ 'Beinbilanz A4-SI .+: \[ROT\]' = 'der rote Test macht das Bein rot - ROT geht vor'; 'Panik in volumenentscheidung' = 'Meldung desselben Tests' } }

# --- M-70: Urteilsvorrang (A36, Quelltextwache) ---
Spec @{ Zeile = 'M-70'; Teil = '(1) Zweige ROT und fehlende Voraussetzung vertauscht'; Bein = $A36Bein
    Zusage = 'ROT gewinnt: Exit 2 vor 3 vor 4; NOT RUN zaehlt nie als gruen.'
    Dateien = @(@{ Datei = 'tools/beweise.ps1'; Ersetzungen = @(@{
Alt = @'
if ($rot -gt 0) {
    $exitcode = 2
    $urteil = "ROT - $rot von $($gelaufen.Count) Kanon-Laeufen fehlgeschlagen$nachsatz"
}
elseif ($fehlendeVoraussetzung -gt 0) {
    $exitcode = 3
    $urteil = "UNVOLLSTAENDIG - $($gruen.Count) gruen, $fehlendeVoraussetzung Voraussetzung(en) fehlen$nachsatz"
}
'@
Neu = @'
if ($fehlendeVoraussetzung -gt 0) {
    $exitcode = 3
    $urteil = "UNVOLLSTAENDIG - $($gruen.Count) gruen, $fehlendeVoraussetzung Voraussetzung(en) fehlen$nachsatz"
}
elseif ($rot -gt 0) {
    $exitcode = 2
    $urteil = "ROT - $rot von $($gelaufen.Count) Kanon-Laeufen fehlgeschlagen$nachsatz"
}
'@ }) })
    Laeufe = @((Lauf-A36 'M-70'), (Lauf-A36 '')); OhneBasislauf = 'entfaellt - Regressionswache der bestehenden Reihenfolge.'
    BauGrund = 'die Quelltextwache liest tools/beweise.ps1 beim Lauf; der Runner selbst laeuft unter dieser Mutation nicht.'
    Erwartet = @((E '^tools/beweise\.ps1: ROT vor fehlender Voraussetzung' 'urteilsvorrang_rot_vor_nicht_gelaufen: tools/beweise.ps1 haelt die Reihenfolge'))
    Mit = @{ 'Gegenteil: dieselbe Quelle mit vertauschten Zweigen' = 'die Gegenprobe des Falls vertauscht die Zweige der schon vertauschten Quelle zurueck - erwartet' } }

Spec @{ Zeile = 'M-70'; Teil = '(2) [NOT RUN] in der Gruenzaehlung'; Bein = $A36Bein
    Zusage = 'NOT RUN zaehlt nie als gruen.'
    Dateien = @(@{ Datei = 'tools/beweise.ps1'; Ersetzungen = @(@{
Alt = @'
$gruen = @($gelaufen | Where-Object { $_.Symbol -in @('[OK]', '[HINWEIS]') })
'@
Neu = @'
$gruen = @($gelaufen | Where-Object { $_.Symbol -in @('[OK]', '[HINWEIS]', '[NOT RUN]') })
'@ }) })
    Laeufe = @((Lauf-A36 'M-70'), (Lauf-A36 '')); OhneBasislauf = 'entfaellt - Regressionswache.'
    BauGrund = 'die Quelltextwache liest tools/beweise.ps1 beim Lauf.'
    Erwartet = @((E '^tools/beweise\.ps1: ROT vor fehlender Voraussetzung' 'urteilsvorrang_rot_vor_nicht_gelaufen: [NOT RUN] fehlt in der Gruenzaehlung'))
    Mit = @{ 'Gegenteil: dieselbe Quelle mit vertauschten Zweigen' = 'die Wache faellt an der Gruenzaehlung auch in der vertauschten Quelle - erwartet'; 'Gegenteil: \[NOT RUN\] in der Gruenzaehlung' = 'die Gegenprobe ersetzt die Originalzeile, die es unter der Mutation nicht mehr gibt - erwartet' } }

# --- M-71, M-72: Fingerprint (B22) ---
$M71Mutation = @{ Datei = 'eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h'; Ersetzungen = @(@{
Alt = @'
            energie += s.psd[(std::size_t) k];
'@
Neu = @'
            energie += s.psd[(std::size_t) k] * ((fpFenster == 0 && (b / kProGruppe) % 2 == 0 && s.ringM[(std::size_t) (s.punkte / 2)] < 0.0) ? 100.0 : 1.0); // MUTATION: vorzeichenabhaengiger Faktor aus dem Frame
'@ }) }
Spec @{ Zeile = 'M-71'; Teil = '(1) vorzeichenabhaengiger Faktor im Fingerprint'; Bein = $B22Bein
    Zusage = 'Dasselbe Material mit umgekehrtem Vorzeichen ergibt bytegleich denselben Fingerprint (alle 76 Byte und der Fensterzaehler).'
    Dateien = @($M71Mutation)
    Bau = @('EqCopSonde013FingerprintGoldenTest'); Laeufe = @((Lauf-Cpp 'B22'))
    OhneBasislauf = 'Teil (2) dieser Datei: dieselbe Mutation gegen den Test am Basis-SHA.'
    Erwartet = @((E '^\[ROT\]\s+M-71: dasselbe Material mit umgekehrtem Vorzeichen' 'M-71: dasselbe Material mit umgekehrtem Vorzeichen ergibt BYTEGLEICH denselben Fingerprint'))
    Hinweis = 'Die Matrixzeile nennt s.arbeit[0] als Quelle des Vorzeichens; s.arbeit haelt an dieser Stelle die FFT des Seitenkanals, der im Testsignal (L = R) null ist. Das Vorzeichen traegt der Mittenring s.ringM (Zeitbereich, Probe 2048 des ersten Fensters). Ein Faktor, der jedes Fenster und jedes Band gleich traefe, verschwaende in der Normierung je Verlauf; die Mutation wirkt deshalb auf die geraden Bandgruppen des ersten Fensters. Am Material A (Probe 2048 positiv) aendert sie nichts, am vorzeichengekehrten Material verstaerkt sie diese Gruppen.' }

Spec @{ Zeile = 'M-71'; Teil = '(2) Gegenprobe: dieselbe Mutation gegen den Test am Basis-SHA'; Bein = $B22Bein; Gegenprobe = $true
    Zusage = 'Behauptung heute rot: pruefe (true, ...) misst nichts - dieselbe Mutation laesst B22 am Basisstand des Tests gruen.'
    Dateien = @($M71Mutation, @{ Datei = 'eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp'; Basisinhalt = $true })
    Bau = @('EqCopSonde013FingerprintGoldenTest'); Laeufe = @((Lauf-Cpp 'B22'))
    OhneBasislauf = 'dies ist der Lauf gegen den Basisstand des Tests (Sonde013FingerprintGoldenTest.cpp am Basis-SHA 8d573e9f).' }

Spec @{ Zeile = 'M-72'; Teil = 'fingerprintJetzt liefert konstante Bytes'; Bein = $B22Bein
    Zusage = 'Material A gegen einen Sinus derselben Grundfrequenz: die Fingerprints unterscheiden sich in mindestens einem Byte.'
    Dateien = @(@{ Datei = 'eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h'; Ersetzungen = @(@{
Alt = @'
    f.gesetzt = true;
'@
Neu = @'
    for (auto& b : f.bandEnergie) b = 128; for (auto& b : f.chroma) b = 128; for (auto& b : f.onset) b = 128; // MUTATION: konstante Bytes
    f.gesetzt = true;
'@ }) })
    Bau = @('EqCopSonde013FingerprintGoldenTest'); Laeufe = @((Lauf-Cpp 'B22')); OhneBasislauf = 'entfaellt - Regressionswache.'
    Erwartet = @((E '^\[ROT\]\s+M-72:' 'M-72: ein Sinus derselben Grundfrequenz ... unterscheidet sich in mindestens einem Byte'))
    Mit = @{ 'anderer Akkord ist messbar unaehnlicher|Rauschen ist einem Akkord|gehoert zu Material B|er ist der von B' = 'konstante Bytes machen jeden Vergleich gleich - dieselbe Ursache, andere Faelle desselben Beins'; 'vor der Grenze traegt die Engine' = 'unberuehrt erwartet, falls hier: konstante Bytes'; 'Sinus und ein Dreieck' = 'unberuehrt erwartet' }
    Hinweis = 'M-71 bleibt unter dieser Mutation gruen (beide Fingerprints sind konstant und gleich): der Trennschaerfefall M-72 ist genau die Probe dagegen.' }

# --- M-75: gemessene Ziele (A36) ---
Spec @{ Zeile = 'M-75'; Teil = '(1) Population der gemessenen Ziele leer'; Bein = $A36Bein
    Zusage = 'Jedes Ziel, das der Runner baut und ein Bein ausfuehrt oder misst, bekommt dasselbe Frischeurteil wie ein Pruefbinary.'
    Dateien = @(@{ Datei = 'tools/eq-copilot/pruefe_beweisrunner.py'; Ersetzungen = @(@{
Alt = @'
        if ziel in beine:
'@
Neu = @'
        if True:  # MUTATION: die gemessenen Ziele entfallen aus der Population
'@ }) })
    Laeufe = @((Lauf-A36 'M-75'), (Lauf-A36 '')); OhneBasislauf = $PyOhneBasis
    Erwartet = @((E '^\(a\) eine Eingabe nur des gemessenen Ziels' '(a) eine Eingabe nur des gemessenen Ziels juenger: VERALTET, Exit 4'))
    Mit = @{ '^Gegenteil: die gemessenen Ziele' = 'ohne Population steht kein gemessenes Ziel in der Tabelle'; '^\(b\)|^\(c\)|^\(d\)' = 'dieselbe Population traegt auch die Faelle nicht ableitbar, fehlt im Export und nicht gebaut' } }

Spec @{ Zeile = 'M-75'; Teil = '(2) Runner fuehrt die gemessenen Ziele nicht in bau_ziele'; Bein = $A36Bein
    Zusage = 'Die Ziele aus $gemesseneZiele gehen ueber $zuBauen in bau_ziele der Frischepruefung.'
    Dateien = @(@{ Datei = 'tools/beweise.ps1'; Ersetzungen = @(@{
Alt = @'
$zuBauen += @($gemesseneZiele | Where-Object { $cmakeText -match [regex]::Escape($_.Marker) } | ForEach-Object { $_.Ziel })
'@
Neu = @'
# MUTATION: gemessene Ziele nicht zugefuehrt
'@ }) })
    Laeufe = @((Lauf-A36 'M-75'), (Lauf-A36 '')); OhneBasislauf = $PyOhneBasis
    BauGrund = 'die Quelltextwache liest tools/beweise.ps1 beim Lauf; der Runner selbst laeuft unter dieser Mutation nicht.'
    Erwartet = @((E '^\(e\) tools/beweise\.ps1: \$gemesseneZiele geht' '(e) tools/beweise.ps1: $gemesseneZiele geht ueber $zuBauen in bau_ziele'))
    Mit = @{ '^Gegenteil: ohne die Zufuehrung' = 'die Gegenprobe entfernt eine Zeile, die es unter der Mutation schon nicht mehr gibt - erwartet' } }

# ---------------------------------------------------------------- Einzelbeine
function Fahre-Einzelbeine {
    $z = New-Object System.Collections.Generic.List[string]
    $kopf = (& git -C $Repo rev-parse --short=8 HEAD).Trim()
    $z.Add('NAK-309 Etappe 4 - einzeln gefahrene Beine (Auftrag Schritt 4: B2, A12, B10, A4, A4-SI, B22, A36)')
    $z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}; git status der Beinquellen: {2}" -f (Get-Date), $kopf, $(if (& git -C $Repo status --porcelain --untracked-files=no) { 'NICHT sauber' } else { 'sauber' })))
    $z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe4-rot.ps1 -Einzelbeine (SHA-256 $WerkzeugSha)")
    $z.Add('A4 und A4-SI im Verfahren des Runners: frischer Meldeordner nur in der Umgebung des Kindprozesses, danach --nicht-gelaufen.')
    $z.Add('')
    $z.Add('--- BAU ---')
    $null = Baue @('EqCopStateMigrationTest', 'EqCopIpcTest', 'EqCopSonde013FingerprintGoldenTest') $null $z
    foreach ($bein in @(
        @{ Name = 'B2 EqCopStateMigrationTest'; Lauf = (Lauf-Cpp 'B2') },
        @{ Name = 'A12 erzeuge_state_fixtures.py --pruefen'; Lauf = (Lauf-A12) },
        @{ Name = 'B10 EqCopIpcTest'; Lauf = (Lauf-Cpp 'B10') },
        @{ Name = 'A4 cargo test (Meldeweg)'; Lauf = (Lauf-Meldeweg @() 'A4' '[OK]') },
        @{ Name = 'A4-SI cargo test --test store_crash_matrix -- --ignored --test-threads=1 (Meldeweg)'; Lauf = (Lauf-Meldeweg @('--test', 'store_crash_matrix', '--', '--ignored', '--test-threads=1') 'A4-SI' '[OK]') },
        @{ Name = 'B22 EqCopSonde013FingerprintGoldenTest'; Lauf = (Lauf-Cpp 'B22') },
        @{ Name = 'A36 pruefe_beweisrunner.py --selbsttest'; Lauf = (Lauf-A36 '') })) {
        $n = Fahre-Lauf $bein.Lauf
        $z.Add('')
        $z.Add("--- $($bein.Name) ---")
        $z.Add("Aufruf: $($n.Aufruf)")
        $z.Add("Exitcode: $($n.Code) ($($n.Sekunden) s)")
        $z.Add("Pruefungen ok: $($n.Ok.Count), rot: $($n.Rot.Count)")
        foreach ($r in $n.Rot) { $z.Add("  $r") }
        foreach ($r in $n.Summe) { $z.Add("  $r") }
        foreach ($l in @($n.Ok | Where-Object { $_ -match 'M-5[78] main-binding|welcome_minor_grenze|M-7[12]:|nicht_gelaufen_ohne_meldeweg_scheitert|nach_join_frist_endet|Beinbilanz|^\((a|b|c|d|e)\) ' })) { $z.Add("  $l") }
    }
    $roh = Join-Path $RohOrdner 'NAK-309-etappe4-einzelbeine.txt'
    [IO.File]::WriteAllText($roh, [string]::Join("`n", $z) + "`n", $Utf8)
    Write-Host "Einzelbeine geschrieben: $(Rel $roh)"
}

# ---------------------------------------------------------------- Einstieg
if ($Einzelbeine) { $script:Cmake = Finde-CMake; Fahre-Einzelbeine; exit 0 }
$script:Cmake = if ($Trocken) { $null } else { Finde-CMake }
$script:Geschrieben = New-Object System.Collections.Generic.HashSet[string]
$auswahl = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$fehler = 0
foreach ($s in $S) {
    if ($auswahl.Count -gt 0 -and $auswahl -notcontains $s.Zeile) { continue }
    foreach ($feld in @('Bau', 'BauGrund', 'OhneBasislauf', 'Mit', 'Hinweis', 'Gegenprobe', 'Vorlauf', 'NachLaeufe', 'OhneMutation', 'Erwartet')) {
        if (-not $s.PSObject.Properties[$feld]) { $s | Add-Member -NotePropertyName $feld -NotePropertyValue $null }
    }
    try { Rotbeweis $s }
    catch { $fehler++; Write-Host ("{0} {1}: WERKZEUGFEHLER {2}" -f $s.Zeile, $s.Teil, $_.Exception.Message) }
}
exit $(if ($fehler -gt 0) { 2 } else { 0 })
