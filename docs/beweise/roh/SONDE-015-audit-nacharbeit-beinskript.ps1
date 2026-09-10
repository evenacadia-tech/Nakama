# Die Beine der Audit-Nacharbeit, einzeln gefahren (kein voller Kanon), nach dem
# Muster von docs/beweise/roh/SONDE-015-etappe4a-beine.txt.
$ErrorActionPreference = 'Continue'
$wurzel = 'C:\Users\phili\Projekte\Nakama'
Set-Location $wurzel
$roh = Join-Path $wurzel 'docs\beweise\roh\SONDE-015-audit-nacharbeit-beine.txt'
$baulog = 'C:\Users\phili\.claude\jobs\3f9ec2f8\tmp\beine-bau.log'
$aus = New-Object System.Collections.Generic.List[string]
$uebersicht = New-Object System.Collections.Generic.List[string]

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

$basis = '07dfcb77c14f82ab3b3c77f7d277474458937520'
$head = (git rev-parse HEAD).Trim()
# Geaenderte Quellen dieses Arbeitsstands plus die Quellen, die nur ein Rotbeweis
# mutierte und bytegleich zurueckspielte (SourcesModel.cpp und WireEnvelope.h fuer
# K-47, DspKern.cpp fuer M-120 Verlauf).
$kandidaten = @(git diff --name-only) + @(git ls-files --others --exclude-standard) + @('eq-copilot/plugin/src/SourcesModel.cpp', 'eq-copilot/plugin/core/ipc/WireEnvelope.h', 'eq-copilot/plugin/dsp/DspKern.cpp')
$quellen = $kandidaten | Where-Object { $_ -match '\.(cpp|h|py|rs|json)$' -and (Test-Path -LiteralPath $_) } | Sort-Object -Unique
$jung = $quellen | ForEach-Object { Get-Item -LiteralPath $_ } | Sort-Object LastWriteTime | Select-Object -Last 1

$ziele = @('EqCopSonde014BefundTest', 'EqCopDspGoldenTest', 'EqCopStateMigrationTest', 'EqCopSchemaTest',
           'EqCopNullTest', 'EqCopProbeeqNullTest', 'EqCopTransactionTest', 'NakamaKern',
           'NakamaProbeeq', 'NakamaProbeeq_VST3', 'EqCopilot', 'EqCopilot_VST3')

$aus.Add('SONDE-015 Audit-Nacharbeit D8/D10/N-12/N-9 - die Beine, einzeln gefahren (kein voller Kanon)')
$aus.Add('Stand: ' + (Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))
$aus.Add('Basis-SHA: ' + $basis + '; HEAD beim Lauf: ' + $head + ' (Arbeitsstand noch nicht committet)')
$aus.Add('Gebaut aus pwsh (cmake liegt nicht im Bash-PATH; CLAUDE.md Maschinen-Landminen), alle Ziele in einem Aufruf:')
$aus.Add('  cmake --build eq-copilot/build --config Release --target ' + ($ziele -join ' '))
$start = Get-Date
& $cmake --build eq-copilot/build --config Release --target @ziele *> $baulog
$bauExit = $LASTEXITCODE
$aus.Add('Bau: Exit ' + $bauExit + ', ' + [int]((Get-Date) - $start).TotalSeconds + ' s')
$aus.Add('Juengste geaenderte oder zurueckgespielte Quelle: ' + $jung.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss') + ' (' + $jung.FullName.Substring($wurzel.Length + 1) + ')')
$aus.Add('Gemessene Quellen (Arbeitsstand, neue Fixtures, SourcesModel.cpp aus dem Rotbeweis K-47):')
foreach ($q in $quellen) { $aus.Add('  ' + $q + '  ' + (Get-Item -LiteralPath $q).LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss')) }
$aus.Add('git status --short beim Lauf:')
foreach ($z in @(git status --short)) { $aus.Add('  ' + $z) }
$aus.Add('Arbeitsverzeichnis jedes Beins: die Repo-Wurzel (wie tools/beweise.ps1).')
$aus.Add('')

function Letzte([string[]] $zeilen) {
    $l = @($zeilen | Where-Object { $_.Trim().Length -gt 0 })
    if ($l.Count -eq 0) { return '' }
    return $l[$l.Count - 1].Trim()
}

function Bein([string] $kuerzel, [string] $titel, [string] $befehlText, [scriptblock] $lauf, [string] $binary) {
    $aus.Add('== ' + $kuerzel + ' ' + $titel + ' ==')
    $aus.Add('Befehl: ' + $befehlText)
    $juenger = '-'
    if ($binary) {
        $zeit = (Get-Item -LiteralPath $binary).LastWriteTime
        $juenger = $(if ($zeit -gt $jung.LastWriteTime) { 'ja' } else { 'NEIN' })
        $aus.Add('Binary: ' + $zeit.ToString('yyyy-MM-dd HH:mm:ss') + ' (juenger als die juengste Quelle: ' + $juenger + ')')
    }
    $start = Get-Date
    $zeilen = @(& $lauf 2>&1 | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    $aus.Add('Exitcode: ' + $code)
    $aus.Add('Dauer: ' + [math]::Round(((Get-Date) - $start).TotalSeconds, 2) + ' s')
    foreach ($z in $zeilen) { $aus.Add($z) }
    $aus.Add('')
    $uebersicht.Add('| ' + $kuerzel + ' | ' + $titel + ' | ' + $code + ' | ' + $juenger + ' | ' + (Letzte $zeilen) + ' |')
}

function Plugin([string] $kuerzel, [string] $name) {
    $exe = 'eq-copilot\build\plugin\' + $name + '_artefacts\Release\' + $name + '.exe'
    Bein $kuerzel $name $exe { & (Join-Path $wurzel $exe) } (Join-Path $wurzel $exe)
}

if ($bauExit -eq 0) {
    Plugin 'B28' 'EqCopSonde014BefundTest'
    Plugin 'B6'  'EqCopDspGoldenTest'
    Plugin 'B2'  'EqCopStateMigrationTest'
    Plugin 'B3c' 'EqCopSchemaTest'
    Plugin 'A1'  'EqCopNullTest'
    Plugin 'A16' 'EqCopProbeeqNullTest'
    Plugin 'B7'  'EqCopTransactionTest'
    Bein 'A12' 'erzeuge_state_fixtures.py' 'py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen' { py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen } $null
    Bein 'A4' 'broker' 'cargo test --manifest-path broker/Cargo.toml --color never' { cargo test --manifest-path broker/Cargo.toml --color never } $null
    # A14 zuletzt: es loescht NakamaKern.lib und laesst MSBuild den Kern neu bauen.
    Bein 'A14' 'pruefe_kern_identitaetsfrei.py' 'py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py' { py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py } $null
} else {
    $aus.Add('BAU GESCHEITERT - kein Bein gefahren; Log ' + $baulog)
}

$aus.Add('== Uebersicht ==')
$aus.Add('| Bein | Ziel | Exit | Binary juenger | Letzte Zeile |')
$aus.Add('|---|---|---|---|---|')
foreach ($z in $uebersicht) { $aus.Add($z) }

$utf8 = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($roh, (($aus -join "`n") + "`n"), $utf8)
foreach ($z in $uebersicht) { Write-Output $z }
Write-Output ('BAU=' + $bauExit)
