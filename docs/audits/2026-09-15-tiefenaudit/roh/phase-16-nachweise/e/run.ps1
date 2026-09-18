param(
    [string]$SourceRoot = 'C:\na-audit16-aff2d818',
    [string]$BuildRoot = 'C:\na-audit16-aff2d818\build-e',
    [string]$CMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    [switch]$RecordExisting,
    [int]$ExistingRunExitCode = -1
)
$ErrorActionPreference = 'Stop'
$ExpectedHead = 'aff2d8188f33a9525aec1869449773bfdbe305a6'
$Head = (& git -C $SourceRoot rev-parse HEAD).Trim()
if ($Head -ne $ExpectedHead) { throw "Anderer Produktstand: $Head" }
& git -C $SourceRoot diff --quiet HEAD -- eq-copilot
if ($LASTEXITCODE -ne 0) { throw 'Produktquellen sind gegen HEAD veraendert' }
$Project = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\c'))
$Binary = Join-Path $BuildRoot 'Audit16Last_artefacts\Release\Audit16Last.exe'
$StartedUtc = (Get-Date).ToUniversalTime().ToString('o')
if (-not $RecordExisting) {
    & $CMake -S $Project -B $BuildRoot -G 'Visual Studio 17 2022' -A x64 "-DAUDIT_REPO=$SourceRoot" "-DFETCHCONTENT_SOURCE_DIR_JUCE=$SourceRoot/deps/juce-src" "-DFETCHCONTENT_SOURCE_DIR_FLATBUFFERS=$SourceRoot/deps/flatbuffers-src" *> (Join-Path $PSScriptRoot 'configure-reproduction.log')
    if ($LASTEXITCODE -ne 0) { throw 'CMake-Konfiguration fehlgeschlagen' }
    & $CMake --build $BuildRoot --config Release --target Audit16Last -- /m:4 *> (Join-Path $PSScriptRoot 'build-reproduction.log')
    if ($LASTEXITCODE -ne 0) { throw 'CMake-Bau fehlgeschlagen' }
    $ConcurrentCompilers = @(Get-Process cl,link,cargo,rustc -ErrorAction SilentlyContinue)
    if ($ConcurrentCompilers.Count -gt 0) { throw 'Ein anderer Compiler laeuft; Timing nicht gestartet' }
    & $Binary $PSScriptRoot *> (Join-Path $PSScriptRoot 'ergebnisse.txt')
    $RunExit = $LASTEXITCODE
    & py -3.13 (Join-Path $PSScriptRoot 'summarize.py') $PSScriptRoot *> (Join-Path $PSScriptRoot 'summary-run.txt')
    if ($LASTEXITCODE -ne 0) { throw 'CSV-Auswertung fehlgeschlagen' }
} else {
    if ($ExistingRunExitCode -lt 0) { throw 'Tatsaechlich beobachteten ExistingRunExitCode angeben' }
    $RunExit = $ExistingRunExitCode
}
$Files = @('last_probe.cpp','ergebnisse.txt','resources.csv','stereo48k64.csv','stereo96k64.csv','stereo48k_mixed.csv','mono48k64.csv','offline48k333.csv','summary.json','summarize.py','run.ps1')
$Evidence = foreach($Name in $Files) {
    $Path = Join-Path $PSScriptRoot $Name
    [ordered]@{name=$Name; sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant(); bytes=(Get-Item -LiteralPath $Path).Length}
}
$ProductFiles = @(& git -C $SourceRoot ls-files eq-copilot/plugin | Where-Object { $_ -match '\.(cpp|h)$' })
$Sources = foreach($Name in $ProductFiles) {
    [ordered]@{path=$Name; sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $SourceRoot $Name)).Hash.ToLowerInvariant()}
}
$Manifest = [ordered]@{
    product_head=$Head
    evidence_level='actual processor component; no VST3/FL/ASIO host; no connected broker'
    source_root=$SourceRoot
    product_tree=(& git -C $SourceRoot rev-parse 'HEAD:eq-copilot').Trim()
    build_root=$BuildRoot
    configuration='MSVC x64 Release, C++20'
    recording_started_utc=$StartedUtc
    record_existing_run=[bool]$RecordExisting
    run_exit_code=$RunExit
    binary=$Binary
    binary_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $Binary).Hash.ToLowerInvariant()
    binary_last_write_utc=(Get-Item -LiteralPath $Binary).LastWriteTimeUtc.ToString('o')
    cpu=@(Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores,NumberOfLogicalProcessors)
    isolation='NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3; Gen v2 CreateFileW/WaitNamedPipeW redirected to PID probe name in c/PipeClientIsoliert.cpp'
    shared_cmake_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Project 'CMakeLists.txt')).Hash.ToLowerInvariant()
    pipe_seam_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Project 'PipeClientIsoliert.cpp')).Hash.ToLowerInvariant()
    evidence=$Evidence
    product_sources=$Sources
}
$Manifest | ConvertTo-Json -Depth 8 | Set-Content -Encoding utf8 -LiteralPath (Join-Path $PSScriptRoot 'manifest.json')
& git -C $SourceRoot diff --quiet HEAD -- eq-copilot
if ($LASTEXITCODE -ne 0) { throw 'Produktquellen waehrend des Laufs veraendert' }
Write-Output "RECORDED_EXIT=$RunExit SOURCES=$($Sources.Count)"
exit $RunExit
