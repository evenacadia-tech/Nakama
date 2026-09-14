<#
NAK-286 Etappe 3, Nacharbeit 1 - Rotskript (Manifest docs/beweise/NAK-286.md, §5.1, §33.2 P-15, §34).

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3-nacharbeit1.ps1                 alle Laeufe
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3-nacharbeit1.ps1 -Nur M-43,M-48  nur diese Zeilen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3-nacharbeit1.ps1 -NurPruefen     nur Traegertexte zaehlen

Je Lauf, in dieser Reihenfolge (§5.1):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot3n1).
  2. Mutation genau an der Traegerzeile: jeder Alttext muss genau einmal vorkommen.
  3. Rust: Bein A4 als Einzeltest (cargo test ... -- <Test> --exact --nocapture), ohne LastWriteTime-Schritt.
     C++: LastWriteTime = jetzt (NAK-230), Bau EqCopBriefkastenTest, Binary juenger als die Quelle, Bein B30 roh.
  4. Rotbeweis: gueltig nur mit Exit ungleich 0 und einer Zeile, die den Traeger der Zusage nennt.
     Messung (Erwartung gruen, kein Rotbeweis): gueltig nur mit Exit 0 und ohne Traegerzeile - sie belegt,
     dass diese Mutation die Zusage nicht bricht (Abweichung in §34).
  5. Bytegleiche Ruecknahme aus den gesicherten Bytes, SHA-256 gleich Schritt 1; C++ zusaetzlich
     LastWriteTime = jetzt, Bau, Binary juenger; derselbe Lauf gruen (Exit 0).
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-43-nach1.txt und NAK-286-rot-M-48-nach1.txt (Rust),
docs/beweise/roh/NAK-286-rot-M-27-nach1.txt (C++), je Lauf ein angehaengter Abschnitt.
Ein ungueltiger Lauf bricht den Lauf nach der Ruecknahme ab.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$tmp = Join-Path $env:TEMP 'nak286-rot3n1'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$exe = Join-Path $wurzel 'eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe'

$BK = 'broker/src/briefkasten.rs'
$CK = 'eq-copilot/plugin/core/diagnose/Briefkasten.cpp'

function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Lauf ([string] $zeile, [string] $teil, [string] $art, [string] $erwartung, [string] $beschreibung, [string] $test, [string] $traeger, [string] $roh, [object[]] $ersetzungen)
{
    [pscustomobject]@{ Zeile = $zeile; Teil = $teil; Art = $art; Erwartung = $erwartung; Beschreibung = $beschreibung; Test = $test; Traeger = $traeger; Roh = $roh; Ersetzungen = $ersetzungen }
}

# Die Laeufe, je an der Zeile, die die Zusage traegt (Rotbeweisspalte der Matrix, §34).
# Art 'rust': Test ist der Name des Unittests in broker/src/briefkasten.rs; Art 'cpp': Bein B30.
$laeufe = New-Object System.Collections.Generic.List[object]

# M-43 (1) und (2): bestehende Rotbeweise der Etappe 3 (§31.2), neu gefahren, weil kennungsregeln_wie_plugin geaendert ist.
$laeufe.Add((Lauf 'M-43' '(1)' 'rust' 'rot' 'Ringabfrage entfernt' 'kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(M-25\) gleiche kennung keine zweite antwort' 'NAK-286-rot-M-43-nach1.txt' @(
    (Ersetzung $BK '        if zustand.ring_enthaelt(&kennung) {' '        if false && zustand.ring_enthaelt(&kennung) { // ROTBEWEIS M-43 (1): Ringabfrage entfernt'))))

$laeufe.Add((Lauf 'M-43' '(2)' 'rust' 'rot' 'Umbenennen mit Ersetzen (MOVEFILE_REPLACE_EXISTING)' 'kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(M-33 \(d\)\) keine datei wird je ersetzt' 'NAK-286-rot-M-43-nach1.txt' @(
    (Ersetzung $BK '        unsafe { MoveFileExW(von.breit.as_ptr(), nach.breit.as_ptr(), MOVEFILE_WRITE_THROUGH) != 0 }' '        unsafe { MoveFileExW(von.breit.as_ptr(), nach.breit.as_ptr(), MOVEFILE_WRITE_THROUGH | windows_sys::Win32::Storage::FileSystem::MOVEFILE_REPLACE_EXISTING) != 0 } // ROTBEWEIS M-43 (2)'))))

# M-43 (3), P-15: die Zeile, die die Objektform erzwingt, entfernt - serde_json belegt die Felder aus dem Array positionell.
$laeufe.Add((Lauf 'M-43' '(3)' 'rust' 'rot' 'Wurzelpruefung entfernt: die Zeile, die die Objektform erzwingt' 'kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(M-27\) fremde anfrage \(i\) JSON-Array mit unbeantworteter Kennung' 'NAK-286-rot-M-43-nach1.txt' @(
    (Ersetzung $BK "    if erstes != Some(b'{') {" "    if false && erstes != Some(b'{') { // ROTBEWEIS M-43 (3): Wurzelpruefung entfernt"))))

# M-43 (4), Ursache (l): die Konstante des Lesers weicht vom Vertrag ab; der Anfragetext der Tests kommt aus der Schemadatei.
$laeufe.Add((Lauf 'M-43' '(4)' 'rust' 'rot' 'Formatkonstante des Lesers weicht von der Vertragsdatei ab (Ursache (l))' 'kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(F-2\) anfrage nach der vertragsdatei gelesen' 'NAK-286-rot-M-43-nach1.txt' @(
    (Ersetzung $BK 'const ANFRAGE_FORMAT: &str = "nakama.diagnose.anfrage.v1";' 'const ANFRAGE_FORMAT: &str = "nakama.diagnose.anfrage.v9"; // ROTBEWEIS M-43 (4): Konstante weicht vom Vertrag ab'))))

# M-48 (Rust-Haelfte): bestehender Rotbeweis der Etappe 3, neu gefahren, weil pfadtraversal_unmoeglich den Anfragetext jetzt aus anfrage_text nimmt.
$laeufe.Add((Lauf 'M-48' '' 'rust' 'rot' 'Kennungsmuster auf 32 beliebige Zeichen gelockert' 'pfadtraversal_unmoeglich' 'M-48 pfadtraversal_unmoeglich \.\.\\waechter\\ \(32 Zeichen\)' 'NAK-286-rot-M-48-nach1.txt' @(
    (Ersetzung $BK "    zeichen.len() == 32 && zeichen.iter().all(|z| matches!(z, b'0'..=b'9' | b'a'..=b'f'))" '    zeichen.len() == 32 // ROTBEWEIS M-48: Kennungsmuster auf 32 beliebige Zeichen gelockert'))))

# M-27 (3) Vorlauf, Messung: Briefkasten.cpp:186 nur gelockert. Der Leser liest danach Schluessel-Wert-Paare; (i) bleibt fremd.
$laeufe.Add((Lauf 'M-27' '(3) Vorlauf' 'cpp' 'gruen' 'Briefkasten.cpp:186 nur gelockert (Pruefung auf { abgeschaltet, ++i bleibt) - Messung, kein Rotbeweis' '' 'M-27 fremde_anfragen_werden_ignoriert \(i\) JSON-Array mit unbeantworteter Kennung' 'NAK-286-rot-M-27-nach1.txt' @(
    (Ersetzung $CK @'
    leer();
    if (i >= n || d[i] != '{')
        return false;
    ++i;
'@ @'
    leer();
    if (false && (i >= n || d[i] != '{'))   // MESSUNG M-27 (3) Vorlauf: Wurzelpruefung nur gelockert
        return false;
    ++i;
'@))))

# M-27 (3), P-15: an Briefkasten.cpp:186 statt der Ablehnung ein Arrayzweig, der die Felder positionell liest wie serde visit_seq.
$laeufe.Add((Lauf 'M-27' '(3)' 'cpp' 'rot' 'Briefkasten.cpp:186 gelockert: ein Array wird positionell gelesen (format, dann anfrage_id) wie serde visit_seq' '' 'M-27 fremde_anfragen_werden_ignoriert \(i\) JSON-Array mit unbeantworteter Kennung' 'NAK-286-rot-M-27-nach1.txt' @(
    (Ersetzung $CK @'
    leer();
    if (i >= n || d[i] != '{')
        return false;
    ++i;
'@ @'
    leer();
    if (i < n && d[i] == '[')   // ROTBEWEIS M-27 (3): Wurzelpruefung gelockert, ein Array wird positionell gelesen
    {
        ++i;
        char feld[64];
        std::size_t feldLaenge = 0;
        leer();
        if (! liesString (d, n, i, feld, sizeof (feld), feldLaenge) || ! gleich (feld, feldLaenge, kAnfrageFormat))
            return false;
        leer();
        if (i >= n || d[i] != ',')
            return false;
        ++i;
        leer();
        if (! liesString (d, n, i, feld, sizeof (feld), feldLaenge) || ! kennungGueltig (feld, feldLaenge))
            return false;
        leer();
        if (i >= n || d[i] != ']')
            return false;
        ++i;
        leer();
        if (i != n)
            return false;
        std::memcpy (aus.z.data(), feld, aus.z.size());
        return true;
    }
    if (i >= n || d[i] != '{')
        return false;
    ++i;
'@))))

function FahreRust ([string] $test, [string] $ausgabe)
{
    $argumente = @('test', '--manifest-path', 'broker/Cargo.toml', '--lib', '--color', 'never', '--', "briefkasten::tests::$test", '--exact', '--nocapture')
    & cargo @argumente *> $ausgabe
    return [pscustomobject]@{ Exit = $LASTEXITCODE; Befehl = 'cargo ' + ($argumente -join ' ') }
}

function Baue ([string] $marke)
{
    $log = Join-Path $tmp "bau-$marke.log"
    & $cmake --build eq-copilot/build --config Release --target EqCopBriefkastenTest *> $log
    return $LASTEXITCODE
}

function FahreB30 ([string] $ausgabe)
{
    $p = Start-Process -FilePath $exe -PassThru -NoNewWindow -RedirectStandardOutput $ausgabe -RedirectStandardError "$ausgabe.err"
    if (-not $p.WaitForExit(900000)) { $p.Kill(); return 99 }
    $p.WaitForExit()
    return $p.ExitCode
}

function JuengerAlsQuellen ([string[]] $dateien)
{
    $quelle = ($dateien | ForEach-Object { (Get-Item (Join-Path $wurzel $_)).LastWriteTime } | Measure-Object -Maximum).Maximum
    return (Get-Item $exe).LastWriteTime -gt $quelle
}

# Mit `pwsh -File` kommt `-Nur M-43,M-27` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($laeufe | Where-Object { $Nur.Count -eq 0 -or $Nur -contains $_.Zeile })
$zusammenfassung = New-Object System.Collections.Generic.List[string]

foreach ($m in $auswahl)
{
    $name = ($m.Zeile + ' ' + $m.Teil).Trim()
    $dateien = @($m.Ersetzungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $orig = @{}; $shaVor = @{}; $texte = @{}
    foreach ($d in $dateien)
    {
        $orig[$d] = [IO.File]::ReadAllBytes((Join-Path $wurzel $d))
        $shaVor[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        [IO.File]::WriteAllBytes((Join-Path $tmp ((Split-Path $d -Leaf) + '.orig')), $orig[$d])
        $texte[$d] = $utf8.GetString($orig[$d])
    }
    $diff = New-Object System.Collections.Generic.List[string]
    foreach ($e in $m.Ersetzungen)
    {
        $t = $texte[$e.Datei]
        $nl = if ($t.Contains("`r`n")) { "`r`n" } else { "`n" }
        $alt = $e.Alt -replace "`r?`n", $nl
        $neu = $e.Neu -replace "`r?`n", $nl
        $n = [regex]::Matches($t, [regex]::Escape($alt)).Count
        if ($n -ne 1) { throw "$name - Traegertext in $($e.Datei) $n-mal statt genau einmal: $($e.Alt)" }
        $i = $t.IndexOf($alt, [StringComparison]::Ordinal)
        $texte[$e.Datei] = $t.Substring(0, $i) + $neu + $t.Substring($i + $alt.Length)
        $diff.Add("Ersetzung in $($e.Datei):")
        foreach ($z in ($e.Alt -split "`r?`n")) { $diff.Add("  - $z") }
        foreach ($z in ($e.Neu -split "`r?`n")) { $diff.Add("  + $z") }
    }
    if ($NurPruefen) { Write-Output "PRUEFUNG OK $name"; continue }

    $shaMut = @{}
    $exitRot = -1; $befehl = ''; $bauRot = -1; $binRot = $true; $rotZeilen = @(); $traeger = @()
    $ausRot = Join-Path $tmp 'lauf-mutiert.txt'
    Remove-Item -LiteralPath $ausRot -ErrorAction SilentlyContinue
    try
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $utf8.GetBytes($texte[$d]))
            if ($m.Art -eq 'cpp') { (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date }
            $shaMut[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        }
        if ($m.Art -eq 'rust')
        {
            $r = FahreRust $m.Test $ausRot
            $exitRot = $r.Exit; $befehl = $r.Befehl
            if (Test-Path $ausRot) { $rotZeilen = @(Get-Content $ausRot) }
        }
        else
        {
            $befehl = "cmake --build eq-copilot/build --config Release --target EqCopBriefkastenTest; $exe"
            $bauRot = Baue (($m.Zeile + $m.Teil) -replace '[^A-Za-z0-9-]', '')
            $binRot = ($bauRot -eq 0) -and (JuengerAlsQuellen $dateien)
            if ($binRot) { $exitRot = FahreB30 $ausRot }
            if (Test-Path $ausRot) { $rotZeilen = @(Get-Content $ausRot | Where-Object { $_.StartsWith('[ROT] ') }) }
        }
        $traeger = @($rotZeilen | Where-Object { $_ -match $m.Traeger })
    }
    finally
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $orig[$d])
            if ($m.Art -eq 'cpp') { (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date }
        }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $ausGruen = Join-Path $tmp 'lauf-gruen.txt'
    Remove-Item -LiteralPath $ausGruen -ErrorAction SilentlyContinue
    $bauGruen = -1; $binGruen = $true
    if ($m.Art -eq 'rust')
    {
        $exitGruen = (FahreRust $m.Test $ausGruen).Exit
        $summeGruen = if (Test-Path $ausGruen) { (Get-Content $ausGruen | Where-Object { $_ -like 'test result:*' } | Select-Object -Last 1) } else { '' }
    }
    else
    {
        $bauGruen = Baue (($m.Zeile + $m.Teil + 'gruen') -replace '[^A-Za-z0-9-]', '')
        $binGruen = ($bauGruen -eq 0) -and (JuengerAlsQuellen $dateien)
        $exitGruen = if ($binGruen) { FahreB30 $ausGruen } else { -1 }
        $summeGruen = if (Test-Path $ausGruen) { (Get-Content $ausGruen | Where-Object { $_ -like 'NAK-286 BRIEFKASTEN:*' } | Select-Object -Last 1) } else { '' }
    }

    $gruende = @()
    if (-not $binRot) { $gruende += "Bau mutiert Exit $bauRot oder Binary nicht juenger" }
    if ($m.Erwartung -eq 'rot')
    {
        if ($exitRot -eq 0 -or $exitRot -eq -1) { $gruende += "Lauf rot Exit $exitRot" }
        if ($traeger.Count -lt 1) { $gruende += 'keine Traegerzeile' }
    }
    else
    {
        if ($exitRot -ne 0) { $gruende += "Messlauf Exit $exitRot statt 0" }
        if ($traeger.Count -ne 0) { $gruende += "Traegerzeilen $($traeger.Count) statt 0" }
    }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if (-not $binGruen) { $gruende += "Bau gruen Exit $bauGruen oder Binary nicht juenger" }
    if ($exitGruen -ne 0) { $gruende += "Lauf gruen Exit $exitGruen" }
    $wort = if ($m.Erwartung -eq 'rot') { 'ROTBEWEIS' } else { 'MESSUNG' }
    $ergebnis = if ($gruende.Count -eq 0) { "$wort GUELTIG" } else { "$wort UNGUELTIG (" + ($gruende -join '; ') + ')' }

    $bein = if ($m.Art -eq 'rust') { 'A4 als Einzeltest' } else { 'B30' }
    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $m.Roh)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 3 Nacharbeit 1, $(if ($m.Erwartung -eq 'rot') { 'Rotbeweis' } else { 'Messung' }) $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Mutation: $($m.Beschreibung)")
    $zeilen.Add("Bein ${bein}: $befehl")
    $zeilen.Add("Traeger (Muster der Meldung): $($m.Traeger)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })")
    }
    if ($m.Art -eq 'cpp') { $zeilen.Add("Bau mutiert: Exit $bauRot, Binary juenger als die Quelle: $(if ($binRot) { 'ja' } else { 'nein' })") }
    $zeilen.Add("$bein mutiert: Exit $exitRot, Traegerzeilen $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add("--- Rohausgabe $bein, mutierter Stand ---")
    if (Test-Path $ausRot) { foreach ($z in (Get-Content $ausRot)) { $zeilen.Add($z) } }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    if ($m.Art -eq 'cpp') { $zeilen.Add("Bau gruen: Exit $bauGruen, Binary juenger als die Quelle: $(if ($binGruen) { 'ja' } else { 'nein' })") }
    $zeilen.Add("$bein gruen: Exit $exitGruen, $summeGruen")
    $zeilen.Add("Ergebnis: $ergebnis")
    $zeilen.Add('')
    # Anhaengen, nie ueberschreiben: ein frueherer Lauf derselben Zeile bleibt als Verlauf stehen.
    [IO.File]::AppendAllText($roh, (($zeilen -join "`n") + "`n"), $utf8)

    $shaText = ($dateien | ForEach-Object { "$(Split-Path $_ -Leaf) $($shaVor[$_].Substring(0,12))=$($shaNach[$_].Substring(0,12))" }) -join ', '
    $zeile = "$name | mutiert Exit $exitRot, Traeger $($traeger.Count) | gruen Exit $exitGruen | $shaText | $ergebnis"
    $zusammenfassung.Add($zeile)
    Write-Output $zeile
    if ($gruende.Count -gt 0) { break }
}

[IO.File]::WriteAllText((Join-Path $tmp 'zusammenfassung.txt'), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE: $($zusammenfassung.Count) Laeufe, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
