param(
    [string]$SourceRoot = 'C:\na-audit16-aff2d818',
    [string]$BuildRoot = 'C:\na-audit16-aff2d818\build-e',
    [switch]$RecordExisting,
    [int]$ExistingRunExitCode = -1
)
$ErrorActionPreference='Stop'
$ExpectedHead='aff2d8188f33a9525aec1869449773bfdbe305a6'
$Head=(& git -C $SourceRoot rev-parse HEAD).Trim()
if($Head -ne $ExpectedHead){throw "Anderer Produktstand: $Head"}
& git -C $SourceRoot diff --quiet HEAD -- eq-copilot
if($LASTEXITCODE -ne 0){throw 'Produktquellen gegen HEAD veraendert'}
$Project=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\c'))
$Binary=Join-Path $BuildRoot 'Audit16LastResources_artefacts\Release\Audit16LastResources.exe'
if(-not $RecordExisting){
    $CMake='C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    & $CMake -S $Project -B $BuildRoot -G 'Visual Studio 17 2022' -A x64 "-DAUDIT_REPO=$SourceRoot" "-DFETCHCONTENT_SOURCE_DIR_JUCE=$SourceRoot/deps/juce-src" "-DFETCHCONTENT_SOURCE_DIR_FLATBUFFERS=$SourceRoot/deps/flatbuffers-src" *> (Join-Path $PSScriptRoot 'configure-resources-reproduction.log')
    if($LASTEXITCODE -ne 0){throw 'CMake-Konfiguration fehlgeschlagen'}
    & $CMake --build $BuildRoot --config Release --target Audit16LastResources -- /m:4 *> (Join-Path $PSScriptRoot 'build-resources-reproduction.log')
    if($LASTEXITCODE -ne 0){throw 'CMake-Bau fehlgeschlagen'}
    & $Binary $PSScriptRoot *> (Join-Path $PSScriptRoot 'resources-run.txt')
    $RunExit=$LASTEXITCODE
}else{
    if($ExistingRunExitCode -lt 0){throw 'Tatsaechlich beobachteten ExistingRunExitCode angeben'}
    $RunExit=$ExistingRunExitCode
}
$Evidence=foreach($Name in @('last_probe.cpp','resources_probe.cpp','resources-run.txt','resources-extended.csv','resources-run.ps1')){
    $Path=Join-Path $PSScriptRoot $Name
    [ordered]@{name=$Name;sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant();bytes=(Get-Item -LiteralPath $Path).Length}
}
$Manifest=[ordered]@{
    product_head=$Head
    product_tree=(& git -C $SourceRoot rev-parse 'HEAD:eq-copilot').Trim()
    evidence_level='actual processor/editor object cycles in one process; no host or connected broker'
    recorded_utc=(Get-Date).ToUniversalTime().ToString('o')
    record_existing_run=[bool]$RecordExisting
    run_exit_code=$RunExit
    completed_cycles=36
    processor_instances=108
    editor_instances=108
    binary=$Binary
    binary_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $Binary).Hash.ToLowerInvariant()
    binary_last_write_utc=(Get-Item -LiteralPath $Binary).LastWriteTimeUtc.ToString('o')
    shared_cmake_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Project 'CMakeLists.txt')).Hash.ToLowerInvariant()
    evidence=$Evidence
}
$Manifest|ConvertTo-Json -Depth 6|Set-Content -Encoding utf8 -LiteralPath (Join-Path $PSScriptRoot 'resources-manifest.json')
& git -C $SourceRoot diff --quiet HEAD -- eq-copilot
if($LASTEXITCODE -ne 0){throw 'Produktquellen waehrend Lauf veraendert'}
Write-Output "RECORDED_RESOURCE_EXIT=$RunExit"
exit $RunExit
