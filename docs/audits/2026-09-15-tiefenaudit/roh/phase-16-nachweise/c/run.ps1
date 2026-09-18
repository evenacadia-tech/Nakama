param([string]$SourceRoot = 'C:\na-audit16-aff2d818', [string]$BuildRoot = 'C:\na-audit16-aff2d818\build-root')
$ErrorActionPreference = 'Stop'
# Der C++-Fixturepfad ist fuer genau diese isolierte Auditkopie eingefroren.
if ([IO.Path]::GetFullPath($SourceRoot) -ne 'C:\na-audit16-aff2d818') { throw 'Dieser eingefrorene Harnisch erwartet C:\na-audit16-aff2d818.' }
$head = (& git -C $SourceRoot rev-parse HEAD).Trim()
if ($head -ne 'aff2d8188f33a9525aec1869449773bfdbe305a6') { throw 'Anderer Produktstand.' }
$productDiff = & git -C $SourceRoot diff HEAD -- broker eq-copilot tools
if ($productDiff) { throw 'Produktquellen sind geaendert.' }
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
& $cmake -S $PSScriptRoot -B $BuildRoot -G 'Visual Studio 17 2022' -A x64 "-DAUDIT_REPO=$SourceRoot" "-DFETCHCONTENT_SOURCE_DIR_JUCE=$SourceRoot/deps/juce-src" "-DFETCHCONTENT_SOURCE_DIR_FLATBUFFERS=$SourceRoot/deps/flatbuffers-src" *> (Join-Path $PSScriptRoot 'configure-rerun.log')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $cmake --build $BuildRoot --config Release --target Audit16State -- /m:4 /nologo *> (Join-Path $PSScriptRoot 'build-rerun.log')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $BuildRoot 'Audit16State_artefacts/Release/Audit16State.exe') *> (Join-Path $PSScriptRoot 'ergebnisse-rerun.txt')
$runCode = $LASTEXITCODE
Get-Content -LiteralPath (Join-Path $PSScriptRoot 'ergebnisse-rerun.txt')
exit $runCode
