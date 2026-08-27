[CmdletBinding()]
param(
    [Parameter()]
    [string] $WorkingDirectory = (Get-Location).Path
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$preflight = Join-Path $PSScriptRoot 'quality-preflight.ps1'

function Assert-Contract {
    param(
        [Parameter(Mandatory)][bool] $Condition,
        [Parameter(Mandatory)][string] $Message
    )

    if (-not $Condition) { throw "quality-preflight self-test failed: $Message" }
}

function Read-PreflightReport {
    param([string] $BaseRef)

    $parameters = @{
        WorkingDirectory = $WorkingDirectory
        Format = 'Json'
    }
    if (-not [string]::IsNullOrWhiteSpace($BaseRef)) {
        $parameters.BaseRef = $BaseRef
    }
    return (& $preflight @parameters | ConvertFrom-Json)
}

$projectRoot = (& git -C $WorkingDirectory rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0) { throw 'git root lookup failed' }
$statusBefore = @(& git -C $projectRoot status --porcelain=v1)
if ($LASTEXITCODE -ne 0) { throw 'initial git status failed' }

$report = Read-PreflightReport
$cmake = @($report.Tools | Where-Object Name -eq 'cmake')
Assert-Contract ($cmake.Count -eq 1) 'the report must contain exactly one CMake entry'
Assert-Contract ([bool]$cmake[0].Required) 'CMake must be required for the canonical build path'
Assert-Contract ([bool]$cmake[0].Available) 'the installed runner CMake must be discovered'
Assert-Contract (Test-Path -LiteralPath $cmake[0].Path -PathType Leaf) 'the reported CMake path must exist'
Assert-Contract ([bool]$report.Canonical.ContextReady) 'the canonical Nakama context must be ready'
Assert-Contract ([bool]$report.Canonical.CanonicalRunnerInvocable) 'the canonical runner must be invocable'

$invalidRef = '__nakama_quality_missing_ref_{0}__' -f [guid]::NewGuid().ToString('N')
$invalidReport = Read-PreflightReport -BaseRef $invalidRef
$invalidFindings = @($invalidReport.Findings | Where-Object Code -eq 'invalid-base-ref')
Assert-Contract (-not [bool]$invalidReport.Canonical.ContextReady) 'an invalid BaseRef must invalidate the context'
Assert-Contract (-not [bool]$invalidReport.Canonical.CanonicalRunnerInvocable) 'an invalid BaseRef must block the runner'
Assert-Contract ($invalidFindings.Count -eq 1) 'an invalid BaseRef must have one named finding'
Assert-Contract ($invalidFindings[0].Message -like "*$invalidRef*") 'the invalid BaseRef finding must name the bad ref'

$oldPath = $env:Path
$oldProgramFiles = $env:ProgramFiles
$oldProgramFilesX86 = ${env:ProgramFiles(x86)}
$tempRoot = Join-Path ([IO.Path]::GetTempPath()) ('nakama-quality-empty-' + [guid]::NewGuid().ToString('N'))
$systemTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$resolvedTempRoot = [IO.Path]::GetFullPath($tempRoot)
Assert-Contract ($resolvedTempRoot.StartsWith($systemTemp, [StringComparison]::OrdinalIgnoreCase)) 'temporary root escaped the system temp directory'
New-Item -ItemType Directory -Path $resolvedTempRoot | Out-Null
try {
    $env:Path = (@($oldPath -split [IO.Path]::PathSeparator | Where-Object {
        $entry = $_.Trim().Trim('"')
        if (-not $entry) { return $false }
        -not (Test-Path -LiteralPath ([IO.Path]::Combine($entry, 'cmake.exe')) -PathType Leaf)
    }) -join [IO.Path]::PathSeparator)
    $env:ProgramFiles = $resolvedTempRoot
    ${env:ProgramFiles(x86)} = $resolvedTempRoot

    $missingReport = Read-PreflightReport
    $missingCMake = @($missingReport.Tools | Where-Object Name -eq 'cmake')
    Assert-Contract ($missingCMake.Count -eq 1) 'the missing-CMake report must contain one CMake entry'
    Assert-Contract (-not [bool]$missingCMake[0].Available) 'the controlled environment must hide CMake'
    Assert-Contract (-not [bool]$missingReport.Canonical.CanonicalRunnerInvocable) 'missing CMake must block the runner'
    Assert-Contract ('required-tool-missing-cmake' -in @($missingReport.Findings.Code)) 'missing CMake must have a named finding'
} finally {
    $env:Path = $oldPath
    $env:ProgramFiles = $oldProgramFiles
    ${env:ProgramFiles(x86)} = $oldProgramFilesX86
    Remove-Item -LiteralPath $resolvedTempRoot -Force
}

$statusAfter = @(& git -C $projectRoot status --porcelain=v1)
if ($LASTEXITCODE -ne 0) { throw 'final git status failed' }
Assert-Contract ([string]::Join("`n", $statusBefore) -eq [string]::Join("`n", $statusAfter)) 'the preflight changed the worktree'

Write-Output 'quality-preflight self-test: PASS'
