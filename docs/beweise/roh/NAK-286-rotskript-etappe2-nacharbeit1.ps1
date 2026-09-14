<#
NAK-286 Etappe 2, Nacharbeit 1 - Rotskript der Matrixzeilen (Manifest docs/beweise/NAK-286.md, §5.1, §24).

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2-nacharbeit1.ps1                   alle Mutationen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2-nacharbeit1.ps1 -Nur M-82,M-34b1  nur diese Kennungen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2-nacharbeit1.ps1 -NurPruefen       nur Traegertexte zaehlen

Kennungen: M-34b1, M-34b2, M-34b3 (Lage (b), §23.2 P-12), M-34a (Lage (a) nach dem Umbau),
M-82, M-83 (§23.2 P-13), M-32, M-36-1, M-36-2, M-54 (vom Umbau beruehrt, erneut gefahren).

Je Mutation, in dieser Reihenfolge (§5.1):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot-n1).
  2. Mutation genau an der Traegerzeile: jeder Alttext muss genau einmal vorkommen.
  3. LastWriteTime = jetzt (NAK-230), Bau EqCopBriefkastenTest, Binary juenger als die Quelle.
  4. Bein B30 roh: gueltig nur mit Exit ungleich 0 und einer [ROT]-Zeile, die den Traeger nennt.
  5. Bytegleiche Ruecknahme aus den gesicherten Bytes, SHA-256 gleich Schritt 1,
     LastWriteTime = jetzt, Bau, Binary juenger, B30 gruen (Exit 0).
Rohausgabe: docs/beweise/roh/NAK-286-rot-<Datei>.txt, je Mutation ein angehaengter Abschnitt.
Eine ungueltige Mutation bricht den Lauf nach der Ruecknahme ab.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$exe = Join-Path $wurzel 'eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe'
$tmp = Join-Path $env:TEMP 'nak286-rot-n1'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)

$BK = 'eq-copilot/plugin/core/diagnose/Briefkasten.cpp'
$AN = 'eq-copilot/plugin/src/prozessor/Analyse.cpp'
$SP = 'eq-copilot/plugin/sonde/SondeProcessor.cpp'

function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Mutation ([string] $kennung, [string] $zeile, [string] $teil, [string] $rohdatei, [string] $beschreibung, [string] $traeger, [int] $mindestens, [object[]] $ersetzungen)
{
    [pscustomobject]@{ Kennung = $kennung; Zeile = $zeile; Teil = $teil; Rohdatei = $rohdatei; Beschreibung = $beschreibung; Traeger = $traeger; Mindestens = $mindestens; Ersetzungen = $ersetzungen }
}

$mutationen = New-Object System.Collections.Generic.List[object]

# ── M-34 Lage (b), §23.2 P-12 ──────────────────────────────────────────────
# Lage (b) laeuft in B30 vor Lage (a): faellt der Kern im Halt, endet der Lauf im Haken,
# bevor ein Takt auf freigegebenem Kern weiterliefe (kein Zugriff auf toten Speicher).
$mutationen.Add((Mutation 'M-34b1' 'M-34' '(b) (1)' 'M-34b' 'Uebergabe der Freigabe an den Message-Thread entfernt: der Halter gibt den Kern auf dem Fremdthread sofort frei' 'M-34 zerstoerung_bei_laufendem_takt \(b\) halt am eintritt des timer-rueckrufs: kern lebt, solange der takt haelt' 1 @(
    (Ersetzung $BK '        juce::MessageManager::callAsync ([rest = std::move (k)]() mutable { rest.reset(); });' '        k.reset();   // ROTBEWEIS M-34 (b) (1): Freigabe nicht an den Message-Thread uebergeben'))))

$mutationen.Add((Mutation 'M-34b2' 'M-34' '(b) (2)' 'M-34b' 'Der Zaehler-Zeuge haelt den Kern (Stand vor dem Umbau): der Halter gibt nicht mehr die letzte Referenz frei' 'M-34 zerstoerung_bei_laufendem_takt \(b\) kern frei, nachdem der test die nachrichten gepumpt hat' 1 @(
    (Ersetzung $BK '    return [werk = kern->zaehlwerk] { return werk->lesen(); };' '    return [werk = kern->zaehlwerk, halter = kern] { (void) halter; return werk->lesen(); };   // ROTBEWEIS M-34 (b) (2): der Zeuge haelt den Kern'))))

$mutationen.Add((Mutation 'M-34b3' 'M-34' '(b) (3)' 'M-34b' 'Freigabe auch auf dem Message-Thread als Nachricht statt sofort' 'M-34 zerstoerung_bei_laufendem_takt gen: auf dem message-thread ist der kern mit dem destruktor frei' 1 @(
    (Ersetzung $BK '        if (! k->timerLief.load() || mm == nullptr || mm->isThisTheMessageThread())' '        if (! k->timerLief.load() || mm == nullptr)   // ROTBEWEIS M-34 (b) (3): auch auf dem Message-Thread per Nachricht'))))

# ── M-34 Lage (a) nach dem Umbau ───────────────────────────────────────────
$mutationen.Add((Mutation 'M-34a' 'M-34' '(a)' 'M-34-nach1' 'Schleuse im Destruktor entfernt' 'M-34 zerstoerung_bei_laufendem_takt angehaltener takt abgewiesen' 1 @(
    (Ersetzung $BK '    k.schleuse.schliessen();' '    // ROTBEWEIS M-34 (a): Schleuse im Destruktor entfernt'))))

# ── M-82 und M-83, §23.2 P-13 ──────────────────────────────────────────────
$mutationen.Add((Mutation 'M-82' 'M-82' '' 'M-82' 'Laden von merkmalFrames hinter den Sperrblock verlegt (der Stand vor der Nacharbeit), hinter den Testhaken' 'M-82 umschlag_kohaerent_gen frames_gebaut gleich der sequenz des kopierten rahmens' 1 @(
    (Ersetzung $AN @'
        auszug.rahmen      = merkmale.frame();
        auszug.framesGebaut = merkmalFrames.load();
'@ @'
        auszug.rahmen      = merkmale.frame();
'@),
    (Ersetzung $AN @'
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
'@ @'
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
    auszug.framesGebaut = merkmalFrames.load();   // ROTBEWEIS M-82: Laden des Zaehlers hinter den Sperrblock verlegt
'@))))

$mutationen.Add((Mutation 'M-83' 'M-83' '' 'M-83' 'Laden von framesGebaut hinter den Sperrblock verlegt (der Stand vor der Nacharbeit), hinter den Testhaken' 'M-83 umschlag_kohaerent_probeeq frames_gebaut gleich der sequenz des kopierten rahmens' 1 @(
    (Ersetzung $SP @'
        auszug.rahmen      = merkmale.frame();
        auszug.framesGebaut = framesGebaut.load();
'@ @'
        auszug.rahmen      = merkmale.frame();
'@),
    (Ersetzung $SP @'
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
'@ @'
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
    auszug.framesGebaut = framesGebaut.load();   // ROTBEWEIS M-83: Laden des Zaehlers hinter den Sperrblock verlegt
'@))))

# ── vom Umbau beruehrte Zeilen, erneut gefahren ────────────────────────────
$mutationen.Add((Mutation 'M-32' 'M-32' '' 'M-32-nach1' 'Rahmenquelle letzterProducerFrame statt merkmale.frame() (Sperrblock mit framesGebaut)' 'M-32 probeeq_antwort_ohne_v3_verbindung frame_aus_der_engine' 1 @(
    (Ersetzung $SP '        auszug.rahmen      = merkmale.frame();' '        auszug.rahmen      = letzterProducerFrame;   // ROTBEWEIS M-32: Rahmenquelle letzterProducerFrame'))))

$mutationen.Add((Mutation 'M-36-1' 'M-36' '(1)' 'M-36-nach1' 'startTimerHz (30) statt startTimer (1000), am Timer des Kerns' 'M-36 kadenz_hoechstens_ein_hertz intervall 1000 ms' 1 @(
    (Ersetzung $BK '        k.startTimer (kTaktMs);' '        k.startTimerHz (30);   // ROTBEWEIS M-36 (1): startTimerHz (30) statt startTimer (1000)'))))

$mutationen.Add((Mutation 'M-36-2' 'M-36' '(2)' 'M-36-nach1' 'Takt auf einem eigenen Thread statt ueber den Timer des Kerns' 'M-36 kadenz_hoechstens_ein_hertz barriere' 1 @(
    (Ersetzung $BK '#include <ctime>' @'
#include <ctime>
#include <thread>   // ROTBEWEIS M-36 (2)
'@),
    (Ersetzung $BK 'struct Briefkasten::Kern final : juce::Timer' 'struct Briefkasten::Kern final : juce::Timer, std::enable_shared_from_this<Briefkasten::Kern>   // ROTBEWEIS M-36 (2)'),
    (Ersetzung $BK @'
        if (hakenTimerEintritt)
            hakenTimerEintritt();
        takt();
'@ @'
        if (hakenTimerEintritt)
            hakenTimerEintritt();
        // ROTBEWEIS M-36 (2): Takt auf einem eigenen Thread statt ueber den Timer
        std::thread ([selbst = shared_from_this()] { selbst->takt(); }).detach();
'@))))

$mutationen.Add((Mutation 'M-54' 'M-54' '' 'M-54-nach1' 'Sperre ueber den Haken zwischen Kopie und Schreiben gehalten (beide Prozessoren, Haken nur im Testbau)' 'M-54 langsamer_takt_ohne_audiowirkung (gen|probeeq): beim schreiben' 2 @(
    (Ersetzung $AN @'
        auszug.bloeckeMax  = material.bloeckeMax();
    }
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // M-54, M-82: der Testhaken hinter dem Sperrblock, vor der Serialisierung.
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
'@ @'
        auszug.bloeckeMax  = material.bloeckeMax();
    // ROTBEWEIS M-54: die Sperre bleibt ueber den Haken hinaus gehalten
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // M-54, M-82: der Testhaken hinter dem Sperrblock, vor der Serialisierung.
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
    }
'@),
    (Ersetzung $SP @'
        auszug.bloeckeMax  = material.bloeckeMax();
    }
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // M-54, M-83: der Testhaken hinter dem Sperrblock, vor der Serialisierung.
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
'@ @'
        auszug.bloeckeMax  = material.bloeckeMax();
    // ROTBEWEIS M-54: die Sperre bleibt ueber den Haken hinaus gehalten
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // M-54, M-83: der Testhaken hinter dem Sperrblock, vor der Serialisierung.
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
#endif
    }
'@))))

function Baue ([string] $marke)
{
    $log = Join-Path $tmp "bau-$marke.log"
    & $cmake --build eq-copilot/build --config Release --target EqCopBriefkastenTest *> $log
    return $LASTEXITCODE
}

function Fahre ([string] $ausgabe)
{
    $p = Start-Process -FilePath $exe -PassThru -NoNewWindow -RedirectStandardOutput $ausgabe -RedirectStandardError "$ausgabe.err"
    if (-not $p.WaitForExit(900000)) { $p.Kill(); return 99 }
    $p.WaitForExit()
    return $p.ExitCode
}

function Zeitstempel ([string[]] $dateien)
{
    $quelle = ($dateien | ForEach-Object { (Get-Item (Join-Path $wurzel $_)).LastWriteTime } | Measure-Object -Maximum).Maximum
    $binary = (Get-Item $exe).LastWriteTime
    return [pscustomobject]@{ Quelle = $quelle; Binary = $binary; Juenger = ($binary -gt $quelle) }
}

$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($mutationen | Where-Object { $Nur.Count -eq 0 -or $Nur -contains $_.Kennung })
$zusammenfassung = New-Object System.Collections.Generic.List[string]
$fmt = 'dd.MM.yyyy HH:mm:ss'

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
        $alt = ($e.Alt -replace "`r?`n", $nl).TrimEnd("`r", "`n")
        $neu = ($e.Neu -replace "`r?`n", $nl).TrimEnd("`r", "`n")
        $n = [regex]::Matches($t, [regex]::Escape($alt)).Count
        if ($n -ne 1) { throw "$name - Traegertext in $($e.Datei) $n-mal statt genau einmal: $($e.Alt)" }
        $i = $t.IndexOf($alt, [StringComparison]::Ordinal)
        $texte[$e.Datei] = $t.Substring(0, $i) + $neu + $t.Substring($i + $alt.Length)
        $diff.Add("Ersetzung in $($e.Datei):")
        foreach ($z in ($e.Alt.TrimEnd("`r", "`n") -split "`r?`n")) { $diff.Add("  - $z") }
        foreach ($z in ($e.Neu.TrimEnd("`r", "`n") -split "`r?`n")) { $diff.Add("  + $z") }
    }
    if ($NurPruefen) { Write-Output "PRUEFUNG OK $($m.Kennung) $name"; continue }

    $shaMut = @{}
    $bauRot = -1; $exitRot = -1; $binRot = $false; $rotZeilen = @(); $traeger = @(); $zeitRot = $null
    $ausRot = Join-Path $tmp 'b30-rot.txt'
    if (Test-Path $ausRot) { Remove-Item $ausRot -Force }
    try
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $utf8.GetBytes($texte[$d]))
            (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date
            $shaMut[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        }
        $bauRot = Baue ($m.Kennung + '-rot')
        $zeitRot = Zeitstempel $dateien
        $binRot = ($bauRot -eq 0) -and $zeitRot.Juenger
        if ($binRot) { $exitRot = Fahre $ausRot }
        if (Test-Path $ausRot)
        {
            $rotZeilen = @(Get-Content $ausRot | Where-Object { $_.StartsWith('[ROT] ') })
            $traeger = @($rotZeilen | Where-Object { $_ -match $m.Traeger })
        }
    }
    finally
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $orig[$d])
            (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date
        }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $bauGruen = Baue ($m.Kennung + '-gruen')
    $zeitGruen = Zeitstempel $dateien
    $binGruen = ($bauGruen -eq 0) -and $zeitGruen.Juenger
    $ausGruen = Join-Path $tmp 'b30-gruen.txt'
    if (Test-Path $ausGruen) { Remove-Item $ausGruen -Force }
    $exitGruen = if ($binGruen) { Fahre $ausGruen } else { -1 }
    $summeGruen = if (Test-Path $ausGruen) { (Get-Content $ausGruen | Where-Object { $_ -like 'NAK-286 BRIEFKASTEN:*' } | Select-Object -Last 1) } else { '' }

    $gruende = @()
    if (-not $binRot) { $gruende += "Bau rot Exit $bauRot oder Binary nicht juenger" }
    if ($exitRot -eq 0 -or $exitRot -eq -1) { $gruende += "Lauf rot Exit $exitRot" }
    if ($traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if (-not $binGruen) { $gruende += "Bau gruen Exit $bauGruen oder Binary nicht juenger" }
    if ($exitGruen -ne 0) { $gruende += "Lauf gruen Exit $exitGruen" }
    $ergebnis = if ($gruende.Count -eq 0) { 'ROTBEWEIS GUELTIG' } else { 'ROTBEWEIS UNGUELTIG (' + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/NAK-286-rot-' + $m.Rohdatei + '.txt')
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 2, Nacharbeit 1, Rotbeweis $name, Kennung $($m.Kennung) ($(Get-Date -Format $fmt)) ===")
    $zeilen.Add("Mutation: $($m.Beschreibung)")
    $zeilen.Add("Traeger (Muster der [ROT]-Zeile, mindestens $($m.Mindestens)): $($m.Traeger)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })")
    }
    $zr = if ($null -ne $zeitRot) { "Quelle $($zeitRot.Quelle.ToString($fmt)), Binary $($zeitRot.Binary.ToString($fmt))" } else { '-' }
    $zeilen.Add("Bau rot: Exit $bauRot, Binary juenger als die Quelle: $(if ($binRot) { 'ja' } else { 'nein' }) ($zr)")
    $zeilen.Add("B30 rot: Exit $exitRot, [ROT]-Zeilen $($rotZeilen.Count), davon Traeger $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add('--- Rohausgabe B30, mutierter Bau ---')
    if (Test-Path $ausRot) { foreach ($z in (Get-Content $ausRot)) { $zeilen.Add($z) } }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    $zeilen.Add("Bau gruen: Exit $bauGruen, Binary juenger als die Quelle: $(if ($binGruen) { 'ja' } else { 'nein' }) (Quelle $($zeitGruen.Quelle.ToString($fmt)), Binary $($zeitGruen.Binary.ToString($fmt)))")
    $zeilen.Add("B30 gruen: Exit $exitGruen, $summeGruen")
    $zeilen.Add("Ergebnis: $ergebnis")
    $zeilen.Add('')
    $text = ($zeilen -join "`n") + "`n"
    [IO.File]::AppendAllText($roh, $text, $utf8)

    $shaText = ($dateien | ForEach-Object { "$(Split-Path $_ -Leaf) $($shaVor[$_].Substring(0,12))=$($shaNach[$_].Substring(0,12))" }) -join ', '
    $zeile = "$($m.Kennung) $name | rot Exit $exitRot, Traeger $($traeger.Count) | gruen Exit $exitGruen | $shaText | $ergebnis"
    $zusammenfassung.Add($zeile)
    Write-Output $zeile
    if ($gruende.Count -gt 0) { break }
}

[IO.File]::WriteAllText((Join-Path $tmp 'zusammenfassung.txt'), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE: $($zusammenfassung.Count) Mutationen, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
