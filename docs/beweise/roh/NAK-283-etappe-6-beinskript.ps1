# NAK-283 Etappe 6 — die Beine der Etappe, einzeln gefahren (Bauer, 13.09.2026)
#
# Beine aus docs/beweise/NAK-283.md §6.7: A24 (voller Soak ueber die
# ausschliessliche Probe-Pipe), A34 (das neue Selbsttestbein), B17, A25, A26,
# A27, A32 (--mit-selbsttest; nicht blockierend, Exit 4 = gerissen), B9, B18.
# Dazu: B27 (C++-Haelfte des Uebergangs M-70), die Vorher-Messung von A32 mit
# dem Werkzeug des Basis-SHA am selben Arbeitsbaum, der frische A24-Bericht
# ueber --bericht (einmal wie er ist, einmal mit dem Mutanten rss_fehlt), der
# historische Kontrollbericht docs/beweise/roh/NAK-246-4ff6f24.md ueber
# --bericht, die Riegel M-66/M-67/M-68/M-70 und dokuriegel.py.
#
# Vor dem Bau: Kanonabgleich (Kuerzel, Name, Art, Ordner und Argumente jedes
# Beins gegen tools/beweise.ps1, PowerShell-AST) und die Pruefung auf fremde
# Bau-, Test- und Kanonprozesse (NAK-290: zwei Kanons teilen die
# Probe-Pipenamen). Nach dem Bau: Frische je Artefakt gegen seine juengste
# Eingabe.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-6-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-6-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
)

$ErrorActionPreference = 'Stop'
Set-Location $Wurzel
$env:PYTHONUTF8 = '1'
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)

$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    foreach ($k in @(
        'C:\Program Files\CMake\bin\cmake.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe')) {
        if (Test-Path $k) { $cmake = $k; break }
    }
}
if (-not $cmake) { throw 'cmake nicht gefunden' }

$build  = Join-Path $Wurzel 'eq-copilot\build'
$roh    = Join-Path $Wurzel 'docs\beweise\roh\NAK-283-etappe-6-beine.txt'
$utf8   = New-Object System.Text.UTF8Encoding $false
$basis  = '7b1f38b7bddcf67965d1bbc2609fd74464cfaaea'
$tmp    = Join-Path ([System.IO.Path]::GetTempPath()) ('nak283-etappe6-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force $tmp | Out-Null
$memory = Join-Path $env:USERPROFILE ('.claude\projects\' + ($Wurzel -replace '[:\\/]', '-') + '\memory')

function Exe-Pfad([string] $ziel) { Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel) }
function Relativ([string] $pfad) { $pfad.Replace($Wurzel + '\', '').Replace('\', '/') }

$ausgabe = New-Object System.Collections.Generic.List[string]
function Notiere([string] $z) { $ausgabe.Add($z); Write-Host $z }

Notiere 'NAK-283 Etappe 6 - Beine (Skript docs/beweise/roh/NAK-283-etappe-6-beinskript.ps1)'
Notiere "Zeitpunkt: $(Get-Date -Format o)"
Notiere "HEAD: $(git -C $Wurzel rev-parse HEAD)"
Notiere "Basis-SHA der Etappe 6: $basis"
Notiere 'git status --short:'
git -C $Wurzel status --short | ForEach-Object { Notiere "  $_" }

# ── Kanonabgleich ─────────────────────────────────────────────────────────
$beine = @(
    @{ K = 'A34'; Art = 'python'; Name = 'pruefe_session_soak.py'; Ordner = 'tools\eq-copilot'; Arg = @('--selbsttest') }
    @{ K = 'B17'; Art = 'plugin'; Name = 'EqCopSonde013TruePeakGoldenTest'; Arg = @() }
    @{ K = 'A25'; Art = 'python'; Name = 'erzeuge_p4_korpus.py'; Ordner = 'tools\eq-copilot'; Arg = @('--pruefen') }
    @{ K = 'A26'; Art = 'python'; Name = 'pruefe_p4_korpus.py'; Ordner = 'tools\eq-copilot'; Arg = @() }
    @{ K = 'A27'; Art = 'python'; Name = 'pruefe_p4_korpus.py'; Ordner = 'tools\eq-copilot'; Arg = @('--selbsttest') }
    @{ K = 'B9';  Art = 'plugin'; Name = 'EqCopLoudnessGoldenTest'; Arg = @() }
    @{ K = 'B18'; Art = 'plugin'; Name = 'EqCopSonde013DynamicsTest'; Arg = @() }
    @{ K = 'B27'; Art = 'plugin'; Name = 'EqCopSonde014IntentTest'; Arg = @() }
    @{ K = 'A32'; Art = 'python'; Name = 'gesundheit.py'; Ordner = 'tools\plan'; Arg = @('--mit-selbsttest') }
    @{ K = 'A24'; Art = 'python'; Name = 'pruefe_session_soak.py'; Ordner = 'tools\eq-copilot'; Arg = @('--sonden', '16', '--minuten', '2', '--neustarts', '1') }
)
$ast = [System.Management.Automation.Language.Parser]::ParseFile((Join-Path $Wurzel 'tools\beweise.ps1'), [ref] $null, [ref] $null)
$kanon = @{}
foreach ($h in $ast.FindAll({ param($n) $n -is [System.Management.Automation.Language.HashtableAst] }, $true)) {
    $w = @{}
    foreach ($kv in $h.KeyValuePairs) {
        $schluessel = [string] $kv.Item1.SafeGetValue()
        try { $w[$schluessel] = $kv.Item2.PipelineElements[0].Expression.SafeGetValue() } catch { $w[$schluessel] = $null }
    }
    if ($w.ContainsKey('Kuerzel') -and $w.ContainsKey('Name')) { $kanon[[string] $w['Kuerzel']] = $w }
}
Notiere 'Kanonabgleich gegen tools/beweise.ps1:'
foreach ($b in $beine) {
    $e = $kanon[$b.K]
    if (-not $e) { throw "Kanonabgleich: $($b.K) steht nicht in tools/beweise.ps1" }
    $ordner = if ($e.ContainsKey('Ordner') -and $e['Ordner']) { [string] $e['Ordner'] } elseif ($e['Art'] -eq 'python') { 'tools\eq-copilot' } else { '' }
    $argKanon = (@($e['Argumente']) -join ' ')
    $argSkript = ($b.Arg -join ' ')
    if ($e['Name'] -ne $b.Name -or $e['Art'] -ne $b.Art -or ($b.Art -eq 'python' -and $ordner -ne $b.Ordner) -or $argKanon -ne $argSkript) {
        throw "Kanonabgleich: $($b.K) weicht ab - Kanon '$($e['Name'])' '$($e['Art'])' '$ordner' '$argKanon', Skript '$($b.Name)' '$($b.Art)' '$($b.Ordner)' '$argSkript'"
    }
    Notiere ("  {0,-4} {1,-6} {2} {3}" -f $b.K, $e['Art'], $(if ($ordner) { "$ordner\$($e['Name'])" } else { $e['Name'] }), $argKanon)
}

# ── Fremde Bau-, Test- und Kanonprozesse (NAK-290) ────────────────────────
function Fremde-Prozesse {
    # Die Befehlszeile zaehlt nur bei Interpretern: eine Claude-Sitzung traegt
    # ihren Auftragstext in der Befehlszeile, und der nennt dieselben Skripte.
    @(Get-CimInstance Win32_Process | Where-Object {
        $_.ProcessId -ne $PID -and (
            $_.Name -match '^(cmake|cl|link|cargo|rustc|eqcop-[^.]*|EqCop[^.]*)\.exe$' -or
            ($_.Name -match '^(pwsh|powershell|python|py)\.exe$' -and $_.CommandLine -and
             $_.CommandLine -match 'beweise\.ps1|laufzeit\.ps1|pruefe_session_soak\.py|pruefe_ipc_last\.py|sonde012_sources_latency'))
    })
}
function Pruefe-Fremde([string] $wann) {
    $fremd = Fremde-Prozesse
    $knoten = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -eq 'MSBuild.exe' })
    Notiere "Fremde Prozesse ($wann): $($fremd.Count); MSBuild-Knoten ohne Bauauftrag: $($knoten.Count)"
    foreach ($p in $fremd) { Notiere "  $($p.ProcessId) $($p.Name) $($p.CommandLine)" }
    if ($fremd.Count -gt 0) { throw "NAK-290: fremder Bau-, Test- oder Kanonprozess laeuft ($wann)" }
}
Pruefe-Fremde 'vor dem Bau'

# ── Bau ───────────────────────────────────────────────────────────────────
$ziele = @('EqCopSonde013TruePeakGoldenTest', 'EqCopSonde013DynamicsTest', 'EqCopLoudnessGoldenTest', 'EqCopSonde014IntentTest', 'EqCopSessionSoak')
$t0 = Get-Date
$bauAus = & $cmake --build $build --config Release --target @ziele 2>&1
$bauExit = $LASTEXITCODE
$bauText = @($bauAus | ForEach-Object { "$_" })
Notiere "Bau: cmake --build eq-copilot/build --config Release --target $($ziele -join ' ') -> Exit $bauExit ($([math]::Round(((Get-Date) - $t0).TotalSeconds, 1)) s)"
if ($bauExit -ne 0) { $bauText | Select-Object -Last 40 | ForEach-Object { Notiere "  $_" }; throw 'Bau fehlgeschlagen' }
$warnungen = @($bauText | Where-Object { $_ -match 'warning C\d{4}' } | ForEach-Object { ($_ -replace '\s+\[[^\]]*\]\s*$', '').Trim() } | Select-Object -Unique)
$inGeaenderter = @($warnungen | Where-Object { $_ -match 'Sonde013TruePeakGoldenTest\.cpp' })
Notiere "  Warnungen (eindeutige Zeilen): $($warnungen.Count), davon in der geaenderten Testdatei Sonde013TruePeakGoldenTest.cpp: $($inGeaenderter.Count)"
foreach ($w in $warnungen) { Notiere "    $w" }

$brokerExe = Join-Path $Wurzel 'broker\target\release\eqcop-broker-sonde012-probe.exe'
$brokerNeueste = @(Get-ChildItem -Recurse -File (Join-Path $Wurzel 'broker\src')) + @(Get-Item (Join-Path $Wurzel 'broker\Cargo.toml')) |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not (Test-Path $brokerExe) -or (Get-Item $brokerExe).LastWriteTime -lt $brokerNeueste.LastWriteTime) {
    $c = & cargo build --release --manifest-path (Join-Path $Wurzel 'broker\Cargo.toml') --bin eqcop-broker-sonde012-probe 2>&1
    Notiere "Broker-Probe neu gebaut: cargo build --release --bin eqcop-broker-sonde012-probe -> Exit $LASTEXITCODE"
    if ($LASTEXITCODE -ne 0) { $c | Select-Object -Last 30 | ForEach-Object { Notiere "  $_" }; throw 'Broker-Bau fehlgeschlagen' }
}
Notiere ("Broker-Probe: {0:yyyy-MM-dd HH:mm:ss}, juengste Broker-Quelle {1} {2:yyyy-MM-dd HH:mm:ss} -> frisch" -f (Get-Item $brokerExe).LastWriteTime, (Relativ $brokerNeueste.FullName), $brokerNeueste.LastWriteTime)

# ── Frische je Artefakt ───────────────────────────────────────────────────
$kernLib = Join-Path $build 'plugin\Release\NakamaKern.lib'
$frische = @(
    @{ Ziel = 'EqCopSonde013TruePeakGoldenTest'; Quellen = @('eq-copilot\plugin\tests\Sonde013TruePeakGoldenTest.cpp', 'eq-copilot\plugin\core\analysis\TruePeak.h', $kernLib) }
    @{ Ziel = 'EqCopSonde013DynamicsTest'; Quellen = @('eq-copilot\plugin\tests\Sonde013DynamicsTest.cpp', 'eq-copilot\plugin\core\analysis\TruePeak.h', $kernLib) }
    @{ Ziel = 'EqCopLoudnessGoldenTest'; Quellen = @('eq-copilot\plugin\tests\LoudnessGoldenTestMain.cpp') }
    @{ Ziel = 'EqCopSonde014IntentTest'; Quellen = @('eq-copilot\plugin\tests\Sonde014IntentTest.cpp', 'eq-copilot\plugin\state\NakamaState.cpp', $kernLib) }
    @{ Ziel = 'EqCopSessionSoak'; Quellen = @('eq-copilot\plugin\tests\SessionSoakMain.cpp', 'eq-copilot\plugin\sonde\SondeProcessor.cpp', $kernLib) }
)
foreach ($f in $frische) {
    $exe = Get-Item (Exe-Pfad $f.Ziel)
    $neueste = $f.Quellen | ForEach-Object { if ([System.IO.Path]::IsPathRooted($_)) { Get-Item $_ } else { Get-Item (Join-Path $Wurzel $_) } } |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    $frisch = $exe.LastWriteTime -ge $neueste.LastWriteTime
    Notiere ("Frische {0}: Binary {1:yyyy-MM-dd HH:mm:ss.fff}, juengste Eingabe {2} {3:yyyy-MM-dd HH:mm:ss.fff} -> {4}" -f $f.Ziel, $exe.LastWriteTime, (Relativ $neueste.FullName), $neueste.LastWriteTime, $(if ($frisch) { 'frisch' } else { 'VERALTET' }))
    if (-not $frisch) { throw "$($f.Ziel) ist veraltet" }
}

# ── Laeufe ────────────────────────────────────────────────────────────────
$ergebnisse = New-Object System.Collections.Generic.List[object]
function Lauf([string] $k, [string[]] $cmd, [int] $erwartet, [string] $zweck) {
    $t0 = Get-Date
    $rest = if ($cmd.Count -gt 1) { $cmd[1..($cmd.Count - 1)] } else { @() }
    $aus = & $cmd[0] @rest 2>&1
    $code = $LASTEXITCODE
    $text = (($aus | ForEach-Object { "$_" }) -join "`n")
    $sek = [math]::Round(((Get-Date) - $t0).TotalSeconds, 1)
    $anzeige = (($cmd | ForEach-Object { $_.Replace($Wurzel + '\', '').Replace($tmp, '<tmp>') }) -join ' ')
    $ergebnisse.Add([pscustomobject]@{ K = $k; Befehl = $anzeige; Exit = $code; Erwartet = $erwartet; Sekunden = $sek; Zweck = $zweck; Text = $text })
    Write-Host ("{0,-26} Exit {1} (erwartet {2}), {3} s" -f $k, $code, $erwartet, $sek)
    return $text
}

$py = 'py'
$null = Lauf 'A34' @($py, '-3.13', 'tools\eq-copilot\pruefe_session_soak.py', '--selbsttest') 0 'das neue Selbsttestbein (F13, M-61 bis M-64)'
$null = Lauf 'B17' @((Exe-Pfad 'EqCopSonde013TruePeakGoldenTest')) 0 'True-Peak-Golden mit den Faellen 20 bis 23 (N01, M-65)'
$null = Lauf 'A25' @($py, '-3.13', 'tools\eq-copilot\erzeuge_p4_korpus.py', '--pruefen') 0 'P4-Korpus bytegleich zur Neuerzeugung (M-66)'
$null = Lauf 'A26' @($py, '-3.13', 'tools\eq-copilot\pruefe_p4_korpus.py') 0 'Sammelbein des P4-Exit-Gates (M-66)'
$null = Lauf 'A27' @($py, '-3.13', 'tools\eq-copilot\pruefe_p4_korpus.py', '--selbsttest') 0 'Selbsttest des P4-Sammelbeins (M-66)'
$null = Lauf 'B9' @((Exe-Pfad 'EqCopLoudnessGoldenTest')) 0 'Klasse referenz des P4-Korpus'
$null = Lauf 'B18' @((Exe-Pfad 'EqCopSonde013DynamicsTest')) 0 'Klasse referenz des P4-Korpus'
$null = Lauf 'B27' @((Exe-Pfad 'EqCopSonde014IntentTest')) 0 'C++-Haelfte des Uebergangs M-70 (Etappe 2)'
$null = Lauf 'A32' @($py, '-3.13', 'tools\plan\gesundheit.py', '--mit-selbsttest') 4 'Quellumfang aus dem Inventar (NAK-249, M-69); nicht blockierend, Exit 4 = gerissen'

$altGesundheit = Join-Path $tmp 'gesundheit-basis-7b1f38b7.py'
[System.IO.File]::WriteAllText($altGesundheit, ((git -C $Wurzel show "${basis}:tools/plan/gesundheit.py") -join "`n") + "`n", $utf8)
$null = Lauf 'A32 vorher' @($py, '-3.13', $altGesundheit, '--mit-selbsttest', '--wurzel', $Wurzel, '--memory', $memory) 4 'das Werkzeug des Basis-SHA am selben Arbeitsbaum: die Werte vorher'

Pruefe-Fremde 'vor A24'
$a24 = Lauf 'A24' @($py, '-3.13', 'tools\eq-copilot\pruefe_session_soak.py', '--sonden', '16', '--minuten', '2', '--neustarts', '1') 0 'voller Soak ueber die Probe-Pipe, Speicherpunkte mit Gueltigkeitsmerkmal'
$berichtZeile = @($a24 -split "`n" | Where-Object { $_.StartsWith('Bericht: {') }) | Select-Object -First 1
if ($berichtZeile) {
    $a24Json = Join-Path $tmp 'a24-bericht.json'
    [System.IO.File]::WriteAllText($a24Json, $berichtZeile.Substring('Bericht: '.Length), $utf8)
    $null = Lauf 'A24 --bericht' @($py, '-3.13', 'tools\eq-copilot\pruefe_session_soak.py', '--bericht', $a24Json) 0 'derselbe frische Bericht, nur geurteilt'
    $null = Lauf 'A24 --bericht rss_fehlt' @($py, '-3.13', 'tools\eq-copilot\pruefe_session_soak.py', '--bericht', $a24Json, '--mutant', 'rss_fehlt') 3 'M-61 am echten Bericht: jede tragende Messung durch die eines nicht abfragbaren Prozesses ersetzt'
} else {
    Notiere 'A24 lieferte keine Berichtzeile - die Laeufe ueber --bericht entfallen'
}
$kontrolle = @(Get-Content -Encoding UTF8 (Join-Path $Wurzel 'docs\beweise\roh\NAK-246-4ff6f24.md') | Where-Object { $_.StartsWith('Bericht: {') })
if ($kontrolle.Count -ne 1) { throw "Kontrollbericht: $($kontrolle.Count) Berichtzeilen statt genau einer" }
$kontrollJson = Join-Path $tmp 'nak246-4ff6f24-bericht.json'
[System.IO.File]::WriteAllText($kontrollJson, $kontrolle[0].Substring('Bericht: '.Length), $utf8)
$null = Lauf 'NAK-246 --bericht' @($py, '-3.13', 'tools\eq-copilot\pruefe_session_soak.py', '--bericht', $kontrollJson) 0 'M-64 an der Kontrollquelle: der Altbericht ohne Merkmal bleibt auswertbar'

$riegel = 'docs\beweise\roh\NAK-283-etappe-6-riegel.py'
$null = Lauf 'Riegel M-66' @($py, '-3.13', $riegel, 'M-66') 0 'beide Fallgruppen getrennt im Korpus'
$null = Lauf 'Riegel M-67' @($py, '-3.13', $riegel, 'M-67') 0 'SONDE-013: nur Zuwachs in den Nachweisspalten von M-02 und M-81'
$null = Lauf 'Riegel M-68' @($py, '-3.13', $riegel, 'M-68') 0 'NAK-199-Fixfassung mit dem Wortlaut aus §36'
$null = Lauf 'Riegel M-68 Registerstand' @($py, '-3.13', $riegel, 'M-68', '--registerstand') 1 'die Registerzeile, wie sie heute steht: der Nachtrag ist Dirigentensache'
$null = Lauf 'Riegel M-70' @($py, '-3.13', $riegel, 'M-70') 0 'beide Haelften des Musters Scheitern ohne unterscheidbaren Status'
$null = Lauf 'dokuriegel' @($py, '-3.13', 'tools\plan\dokuriegel.py', 'docs/beweise/SONDE-013.md', 'docs/beweise/NAK-283.md') 0 'die zwei beruehrten Manifeste ohne Befund'

# ── Bericht ───────────────────────────────────────────────────────────────
Notiere ''
Notiere 'Uebersicht (Exit gegen die Erwartung):'
foreach ($e in $ergebnisse) {
    Notiere ('  {0,-26} Exit {1,2} erwartet {2,2} {3,7} s  {4}  - {5}' -f $e.K, $e.Exit, $e.Erwartet, $e.Sekunden, $(if ($e.Exit -eq $e.Erwartet) { 'wie erwartet' } else { 'ABWEICHUNG' }), $e.Befehl)
}
$abweichend = @($ergebnisse | Where-Object { $_.Exit -ne $_.Erwartet })
Notiere ("URTEIL: {0} von {1} Laeufen mit erwartetem Exit" -f ($ergebnisse.Count - $abweichend.Count), $ergebnisse.Count)
foreach ($e in $ergebnisse) {
    $ausgabe.Add('')
    $ausgabe.Add("#### $($e.K) | $($e.Befehl) | Exit $($e.Exit) (erwartet $($e.Erwartet)) | $($e.Sekunden) s | $($e.Zweck)")
    $ausgabe.Add($e.Text)
}
[System.IO.File]::WriteAllLines($roh, $ausgabe, $utf8)
[System.IO.Directory]::Delete($tmp, $true)
if ($abweichend.Count -gt 0) { exit 1 }
exit 0
