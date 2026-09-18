param(
    [string]$ProductRoot = 'C:\na-audit16-aff2d818',
    [string]$JuceRoot = 'C:\na-audit16-aff2d818\deps\juce-src',
    [string]$BuildRoot = 'C:\na-audit16-aff2d818\build-d',
    [string]$CMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
)
$ErrorActionPreference = 'Stop'
$expected = 'aff2d8188f33a9525aec1869449773bfdbe305a6'
$actual = (& git -C $ProductRoot rev-parse HEAD).Trim()
if ($actual -ne $expected) { throw "Wrong product revision: $actual" }
$productPaths = @(
    'eq-copilot/plugin/dsp/DspKern.cpp', 'eq-copilot/plugin/dsp/DspKern.h',
    'eq-copilot/plugin/dsp/DspProgramm.cpp', 'eq-copilot/plugin/dsp/DspProgramm.h',
    'eq-copilot/plugin/dsp/DspBankPool.cpp', 'eq-copilot/plugin/dsp/DspBankPool.h',
    'eq-copilot/plugin/dsp/DspFilter.h', 'eq-copilot/plugin/dsp/DspSvf.h',
    'eq-copilot/plugin/state/NakamaParameter.cpp', 'eq-copilot/plugin/state/NakamaParameter.h',
    'eq-copilot/plugin/state/NakamaKanon.cpp', 'eq-copilot/plugin/vertrag/NakamaVertrag.cpp',
    'eq-copilot/plugin/state/NakamaTransaktion.cpp', 'eq-copilot/plugin/state/NakamaTransaktion.h',
    'eq-copilot/plugin/state/NakamaPreset.cpp'
)
$before = @{}
foreach ($path in $productPaths) { $before[$path] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $ProductRoot $path)).Hash }
$cmakeProductRoot = $ProductRoot.Replace('\', '/')
$cmakeJuceRoot = $JuceRoot.Replace('\', '/')
& $CMake -S $PSScriptRoot -B $BuildRoot -G 'Visual Studio 17 2022' -A x64 "-DNAKAMA_SOURCE=$cmakeProductRoot" "-DJUCE_SOURCE=$cmakeJuceRoot" *> (Join-Path $PSScriptRoot 'configure.txt')
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed: $LASTEXITCODE" }
& $CMake --build $BuildRoot --config Release --target audit16d --parallel 2 *> (Join-Path $PSScriptRoot 'build.txt')
if ($LASTEXITCODE -ne 0) { throw "CMake build failed: $LASTEXITCODE" }
$binary = Join-Path $BuildRoot 'Release\audit16d.exe'
& $binary *> (Join-Path $PSScriptRoot 'ergebnisse.txt')
$probeExit = $LASTEXITCODE
$unchanged = $true
foreach ($path in $productPaths) {
    $after = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $ProductRoot $path)).Hash
    if ($after -ne $before[$path]) { $unchanged = $false }
}
$manifest = [ordered]@{
    productHead = $actual; capturedAtUtc = [DateTime]::UtcNow.ToString('o'); productRoot = $ProductRoot
    buildRoot = $BuildRoot; juceSource = $JuceRoot; probeExit = $probeExit
    binarySha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $binary).Hash
    binaryLastWriteUtc = (Get-Item -LiteralPath $binary).LastWriteTimeUtc.ToString('o')
    productSourcesUnchanged = $unchanged; sourcesSha256 = $before
    evidenceClass = 'unchanged real DSP component execution, not SondeProcessor or VST3 host execution'
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'manifest.json') -Encoding utf8
if (-not $unchanged) { throw 'Product source changed during build/run' }
if ($probeExit -ne 0) { throw "Probe failed: $probeExit" }
Write-Output 'Audit harness finished; EXPECTED_FAIL marks reproduced defects, not a product PASS.'
