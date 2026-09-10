param([string] $Spec)
# Rotbeweis nach dem Muster der Etappe 4a: Mutation an der tragenden Zeile,
# Bau, Lauf (muss ROT sein und an der erwarteten Pruefung fallen), bytegleiche
# Ruecknahme mit SHA-256, Neubau, Lauf (muss GRUEN sein). Die Spezifikation
# liegt als JSON-Datei vor, damit kein mehrzeiliger Text durch die Kommandozeile
# muss (CLAUDE.md, Maschinen-Landminen). Mehrere Mutationen derselben Datei
# werden nacheinander auf denselben Text angewandt und gemeinsam zurueckgespielt.
$ErrorActionPreference = 'Stop'
$wurzel = 'C:\Users\phili\Projekte\Nakama'
Set-Location $wurzel
$s = Get-Content -LiteralPath $Spec -Raw -Encoding utf8 | ConvertFrom-Json

function Finde-CMake {
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
    return $null
}
$cmake = Finde-CMake

function Sha([byte[]] $b) {
    $h = [System.Security.Cryptography.SHA256]::Create()
    return ([System.BitConverter]::ToString($h.ComputeHash($b))).Replace('-', '')
}

function Baue([string] $ziel, [string] $marke) {
    $log = Join-Path $env:TEMP ('rot-bau-' + $s.id + '-' + $marke + '.log')
    $start = Get-Date
    & $cmake --build eq-copilot/build --config Release --target $ziel *> $log
    $code = $LASTEXITCODE
    return [pscustomobject]@{ Exit = $code; Log = $log; Sekunden = [int]((Get-Date) - $start).TotalSeconds }
}

function Laufe([string] $exe, $argumente) {
    $alt = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $start = Get-Date
    $zeilen = & (Join-Path $wurzel $exe) @argumente 2>&1 | ForEach-Object { "$_" }
    $code = $LASTEXITCODE
    $ErrorActionPreference = $alt
    return [pscustomobject]@{ Exit = $code; Zeilen = @($zeilen); Sekunden = [math]::Round(((Get-Date) - $start).TotalSeconds, 1) }
}

$aus = New-Object System.Collections.Generic.List[string]
$argumente = @()
if ($s.argumente) { $argumente = @($s.argumente) }
$utf8 = New-Object System.Text.UTF8Encoding($false)

# --- Mutationen vorbereiten -------------------------------------------------
$dateien = [ordered]@{}
$muts = @()
foreach ($m in $s.mutationen) {
    if (-not $dateien.Contains([string] $m.datei)) {
        $pfad = Join-Path $wurzel $m.datei
        $bytes = [System.IO.File]::ReadAllBytes($pfad)
        $bom = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
        $text = [System.Text.Encoding]::UTF8.GetString($bytes)
        if ($bom) { $text = $text.Substring(1) }
        $dateien[[string] $m.datei] = [pscustomobject]@{ Datei = [string] $m.datei; Pfad = $pfad; Original = $bytes; Hash = (Sha $bytes); OriginalText = $text; Text = $text; Bom = $bom; Crlf = $text.Contains("`r`n") }
    }
    $d = $dateien[[string] $m.datei]
    $vorher = [string] $m.vorher
    $nachher = [string] $m.nachher
    if ($d.Crlf) { $vorher = $vorher.Replace("`n", "`r`n"); $nachher = $nachher.Replace("`n", "`r`n") }
    $erste = $d.OriginalText.IndexOf($vorher, [System.StringComparison]::Ordinal)
    $letzte = $d.OriginalText.LastIndexOf($vorher, [System.StringComparison]::Ordinal)
    if ($erste -lt 0 -or $erste -ne $letzte) { throw ('Mutation nicht eindeutig in ' + $m.datei + ' (erste=' + $erste + ', letzte=' + $letzte + ')') }
    $zeile = $d.OriginalText.Substring(0, $erste).Split("`n").Length
    $i = $d.Text.IndexOf($vorher, [System.StringComparison]::Ordinal)
    if ($i -lt 0) { throw ('Mutationen ueberlappen in ' + $m.datei) }
    $d.Text = $d.Text.Substring(0, $i) + $nachher + $d.Text.Substring($i + $vorher.Length)
    $muts += [pscustomobject]@{ Datei = [string] $m.datei; Zeile = $zeile; Vorher = [string] $m.vorher; Nachher = [string] $m.nachher }
}

$aus.Add([string] $s.ueberschrift)
$aus.Add('Stand:  ' + (Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))
$aus.Add('Zusage: ' + $s.zusage)
$aus.Add('Bein:   ' + $s.bein)
$aus.Add('Datei:  ' + (($muts | ForEach-Object { $_.Datei + ':' + $_.Zeile }) -join ', '))
$aus.Add('')
$aus.Add('--- MUTIERTE ZEILE(N) (die Zeile, die die Zusage traegt) ---')
foreach ($mu in $muts) {
    $aus.Add($mu.Datei + ':' + $mu.Zeile)
    $aus.Add('vorher:')
    foreach ($z in $mu.Vorher.Split("`n")) { $aus.Add($z) }
    $aus.Add('nachher:')
    foreach ($z in $mu.Nachher.Split("`n")) { $aus.Add($z) }
}
$aus.Add('')

# --- Mutation anwenden ------------------------------------------------------
foreach ($d in $dateien.Values) {
    $b = $utf8.GetBytes($d.Text)
    if ($d.Bom) { $b = [byte[]](0xEF, 0xBB, 0xBF) + $b }
    [System.IO.File]::WriteAllBytes($d.Pfad, $b)
    (Get-Item -LiteralPath $d.Pfad).LastWriteTime = Get-Date
}
$jungQuelle = ($dateien.Values | ForEach-Object { (Get-Item -LiteralPath $_.Pfad).LastWriteTime } | Sort-Object | Select-Object -Last 1)

$rotOk = $false
$faelltRichtig = $false
try {
    $bau = Baue $s.ziel 'rot'
    $exeZeit = (Get-Item -LiteralPath (Join-Path $wurzel $s.exe)).LastWriteTime
    $aus.Add('--- LAUF UNTER MUTATION: ROT ---')
    $aus.Add('Bau: Exit ' + $bau.Exit + ' (' + $bau.Sekunden + ' s); Binary ' + $exeZeit.ToString('HH:mm:ss') + ', mutierte Quelle ' + $jungQuelle.ToString('HH:mm:ss') + ', Binary juenger: ' + $(if ($exeZeit -gt $jungQuelle) { 'ja' } else { 'NEIN' }))
    if ($bau.Exit -ne 0) {
        $aus.Add('BAU GESCHEITERT - Log ' + $bau.Log)
        foreach ($z in (Get-Content -LiteralPath $bau.Log | Select-String -Pattern 'error' | Select-Object -First 20)) { $aus.Add('  ' + $z.Line) }
    } else {
        $lauf = Laufe $s.exe $argumente
        $aus.Add('Exitcode: ' + $lauf.Exit + ' (' + $lauf.Sekunden + ' s)')
        foreach ($z in ($lauf.Zeilen | Select-Object -Last 3)) { $aus.Add($z) }
        $aus.Add('')
        $gefallen = @($lauf.Zeilen | Where-Object { $_ -match '^\s*FEHLER\s' })
        $aus.Add('Gefallene Pruefungen (' + $gefallen.Count + '):')
        foreach ($z in $gefallen) { $aus.Add($z) }
        $aus.Add('')
        $faelltRichtig = $true
        foreach ($e in $s.erwartet) {
            if (-not ($gefallen | Where-Object { $_.Contains([string] $e) })) { $faelltRichtig = $false }
        }
        $rotOk = ($lauf.Exit -ne 0) -and ($exeZeit -gt $jungQuelle)
        $aus.Add('Die Zusage faellt an der erwarteten Pruefung: ' + $(if ($faelltRichtig -and $rotOk) { 'JA' } else { 'NEIN' }))
        foreach ($e in $s.erwartet) { $aus.Add('  erwartet: ' + $e) }
        if ($s.auch_zeigen) {
            $aus.Add('')
            $aus.Add('Weitere Zeilen des roten Laufs (zur Einordnung):')
            foreach ($e in $s.auch_zeigen) {
                foreach ($z in @($lauf.Zeilen | Where-Object { $_.Contains([string] $e) })) { $aus.Add($z) }
            }
        }
        if ($s.hinweis) { $aus.Add(''); $aus.Add('Hinweis: ' + $s.hinweis) }
    }
}
finally {
    # --- Ruecknahme: IMMER, bytegleich ------------------------------------
    $aus.Add('')
    $aus.Add('--- RUECKNAHME ---')
    foreach ($d in $dateien.Values) {
        [System.IO.File]::WriteAllBytes($d.Pfad, $d.Original)
        (Get-Item -LiteralPath $d.Pfad).LastWriteTime = Get-Date
        $zurueck = Sha ([System.IO.File]::ReadAllBytes($d.Pfad))
        $aus.Add('Quelle ' + $d.Datei + ' bytegleich zurueckgespielt: ' + $(if ($zurueck -eq $d.Hash) { 'ja' } else { 'NEIN' }) + ' (SHA-256 ' + $zurueck + ')')
    }
}

$jungZurueck = ($dateien.Values | ForEach-Object { (Get-Item -LiteralPath $_.Pfad).LastWriteTime } | Sort-Object | Select-Object -Last 1)
$bau2 = Baue $s.ziel 'gruen'
$exeZeit2 = (Get-Item -LiteralPath (Join-Path $wurzel $s.exe)).LastWriteTime
$aus.Add('')
$aus.Add('--- LAUF NACH RUECKNAHME: GRUEN ---')
$aus.Add('Stand: ' + (Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))
$aus.Add('Bau: Exit ' + $bau2.Exit + ' (' + $bau2.Sekunden + ' s); Binary ' + $exeZeit2.ToString('HH:mm:ss') + ', zurueckgespielte Quelle ' + $jungZurueck.ToString('HH:mm:ss') + ', Binary juenger: ' + $(if ($exeZeit2 -gt $jungZurueck) { 'ja' } else { 'NEIN' }))
$gruenOk = $false
if ($bau2.Exit -eq 0) {
    $lauf2 = Laufe $s.exe $argumente
    $aus.Add('Exitcode: ' + $lauf2.Exit + ' (' + $lauf2.Sekunden + ' s)')
    foreach ($e in $s.erwartet) {
        foreach ($z in @($lauf2.Zeilen | Where-Object { $_.Contains([string] $e) })) { $aus.Add($z) }
    }
    foreach ($z in ($lauf2.Zeilen | Select-Object -Last 2)) { $aus.Add($z) }
    $gruenOk = ($lauf2.Exit -eq 0) -and ($exeZeit2 -gt $jungZurueck)
}
$aus.Add('Gruen nach Ruecknahme: ' + $(if ($gruenOk) { 'JA' } else { 'NEIN' }))

[System.IO.File]::WriteAllText((Join-Path $wurzel $s.ausgabe), (($aus -join "`n") + "`n"), $utf8)
Write-Output ('ROTBEWEIS ' + $s.id + ': rot=' + $rotOk + ' richtig=' + $faelltRichtig + ' gruen=' + $gruenOk)
