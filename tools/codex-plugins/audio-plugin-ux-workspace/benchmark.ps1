#Requires -Version 7.0
[CmdletBinding()]
param(
    [string] $BaselineRoot,
    [string] $CandidateRoot,
    [string] $RepoRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not $BaselineRoot) { $BaselineRoot = Join-Path $PSScriptRoot 'baseline-0.2.1' }
if (-not $CandidateRoot) { $CandidateRoot = Join-Path $PSScriptRoot '../audio-plugin-ux' }
if (-not $RepoRoot) { $RepoRoot = Join-Path $PSScriptRoot '../../..' }

$BaselineRoot = [System.IO.Path]::GetFullPath($BaselineRoot)
$CandidateRoot = [System.IO.Path]::GetFullPath($CandidateRoot)
$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)

$tests = @(
    [pscustomobject]@{
        id = 'E1-indexed-research-route'
        file = 'skills/audio-plugin-ux/SKILL.md'
        require = @('indexed research archive', 'references/research-evidence\.md', 'do not copy its findings')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E2-dynamic-archive-discovery'
        file = 'skills/audio-plugin-ux/references/research-evidence.md'
        require = @('wissen/LIES-MICH\.md', 'wissen/AGENTS\.md', 'wissen/INDEX\.md', 'Do not hard-code today''s dated entry filenames')
        reject = @('2026-08-26-juce-ui-performance-messen')
    },
    [pscustomobject]@{
        id = 'E3-authority-boundaries'
        file = 'skills/audio-plugin-ux/references/research-evidence.md'
        require = @('Source claim', 'Current product contract', 'Transfer inference', 'Design decision', 'Bedeutung für Nakama')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E4-human-causal-chain'
        file = 'skills/audio-plugin-ux/SKILL.md'
        require = @('Human-need chain', 'person and situation', 'perceptual or interaction need', 'observable proof')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E5-contextual-mapping'
        file = 'skills/audio-plugin-ux/references/design-reasoning.md'
        require = @('Mapping compatibility is a hypothesis', 'not universal human laws', 'Mapping-comprehension test')
        reject = @('horizontal position naturally represents frequency')
    },
    [pscustomobject]@{
        id = 'E6-operable-access-routes'
        file = 'skills/audio-plugin-ux/references/design-reasoning.md'
        require = @('coarse and fine increments', 'reset/default', 'malformed or out-of-range', 'Control-usability test')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E7-visual-evidence-provenance'
        file = 'skills/audio-plugin-ux/references/research-evidence.md'
        require = @('page, figure, frame, or video timestamp', 'objective observation', 'source''s own claim', 'separate interpretation')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E8-measure-performance-first'
        file = 'skills/audio-plugin-ux/references/research-evidence.md'
        require = @('build type, OS, host, scale, instance count', 'static and dynamic paint work', 'audio-thread health', 'prompt control feedback', 'idle editors must release host resources')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E9-native-migration-continuity'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @('parameter contract', 'state contract', 'audio contract', 'host contract', 'previous working native slice', 'Windows Release behavior in FL Studio', 'multiple instances')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E10-no-product-snapshot-copy'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @('stable repository router', 'Attach a current repository path to every volatile product claim')
        reject = @('Current product contract to verify, not memorize', 'current visual truth in progress is exactly three images', '8 band slots × 13')
    },
    [pscustomobject]@{
        id = 'E11-progressive-reference-loading'
        file = 'skills/audio-plugin-ux/SKILL.md'
        require = @('explicit FabFilter Pro-Q comparison', 'Do not load it merely because the product is an EQ')
        reject = @()
    },
    [pscustomobject]@{
        id = 'E12-version-and-layout-prompt'
        file = '.codex-plugin/plugin.json'
        require = @('"version":\s*"0\.2\.4"', 'Derive a layout hypothesis from this parameter inventory and fixed host surface')
        reject = @()
    }
)

function Invoke-Suite {
    param([string] $Label, [string] $Root)

    $results = foreach ($test in $tests) {
        $path = Join-Path $Root $test.file
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            [pscustomobject]@{
                id = $test.id
                passed = $false
                detail = "Missing file: $($test.file)"
            }
            continue
        }

        $content = [System.IO.File]::ReadAllText($path)
        $missing = @($test.require | Where-Object { -not [regex]::IsMatch($content, $_, 'IgnoreCase') })
        $forbidden = @($test.reject | Where-Object { [regex]::IsMatch($content, $_, 'IgnoreCase') })
        [pscustomobject]@{
            id = $test.id
            passed = ($missing.Count -eq 0 -and $forbidden.Count -eq 0)
            detail = if ($missing.Count -gt 0) {
                "Missing: $($missing -join '; ')"
            }
            elseif ($forbidden.Count -gt 0) {
                "Forbidden: $($forbidden -join '; ')"
            }
            else {
                'All assertions passed.'
            }
        }
    }

    [pscustomobject]@{
        label = $Label
        root = $Root
        passed = @($results | Where-Object passed).Count
        total = $tests.Count
        results = @($results)
    }
}

function Invoke-ArchiveContract {
    param([string] $PluginRoot, [string] $WorkspaceRoot)

    $checker = Join-Path $PluginRoot 'skills/audio-plugin-ux/scripts/check-nakama-contract.ps1'
    if (-not (Test-Path -LiteralPath $checker -PathType Leaf)) {
        return [pscustomobject]@{
            ok = $false
            detail = 'Candidate contract checker is missing.'
            report = $null
        }
    }

    $output = & pwsh -NoProfile -File $checker -RepoRoot $WorkspaceRoot 2>&1 | Out-String
    $exitCode = $LASTEXITCODE
    try {
        $parsed = $output | ConvertFrom-Json
        [pscustomobject]@{
            ok = ($exitCode -eq 0 -and $parsed.ok)
            detail = if ($exitCode -eq 0 -and $parsed.ok) {
                "$($parsed.checks.Count) archive and adapter checks passed."
            }
            else {
                "Contract checker failed with $($parsed.failedCount) failed checks."
            }
            report = $parsed
        }
    }
    catch {
        [pscustomobject]@{
            ok = $false
            detail = "Contract checker did not return valid JSON (exit $exitCode): $output"
            report = $null
        }
    }
}

$baseline = Invoke-Suite -Label 'baseline-snapshot-0.2.1' -Root $BaselineRoot
$candidate = Invoke-Suite -Label 'source-candidate-0.2.4' -Root $CandidateRoot
$contract = Invoke-ArchiveContract -PluginRoot $CandidateRoot -WorkspaceRoot $RepoRoot

$report = [pscustomobject]@{
    baseline = $baseline
    candidate = $candidate
    archiveContract = $contract
}

$report | ConvertTo-Json -Depth 8
if ($candidate.passed -ne $candidate.total -or -not $contract.ok) { exit 1 }
exit 0
