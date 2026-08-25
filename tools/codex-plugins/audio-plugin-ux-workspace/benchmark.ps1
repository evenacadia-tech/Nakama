#Requires -Version 7.0
[CmdletBinding()]
param(
    [string] $BaselineRoot = 'C:/Users/phili/plugins/audio-plugin-ux-workspace/baseline-0.2.1',
    [string] $CandidateRoot = 'C:/Users/phili/plugins/audio-plugin-ux'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$tests = @(
    [pscustomobject]@{
        id = 'E1-current-routes'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @(
            'CLAUDE\.md',
            'design/abnahmen/2026-08-23-gen-eq-zentrale\.md',
            'design/docs/ui-spielregeln-eq-zentrale\.md',
            'design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest\.md'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E2-no-stale-product'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @(
            'two apps',
            '760×430',
            '8 band slots × 13',
            'U5/NAK-33 is closed'
        )
        reject = @(
            'Nakama Suna\*\* is the compact passive probe surface',
            'eight bands at 700x420',
            'twelve displayed parameters per band',
            'advises rather than directly processing'
        )
    },
    [pscustomobject]@{
        id = 'E3-generative-fixed-surface'
        file = 'skills/audio-plugin-ux/references/design-reasoning.md'
        require = @(
            'Generate a layout hypothesis from constraints',
            'Measure the real surface',
            'fixed-aspect interface',
            'worst truthful states'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E4-mixed-identity-scales'
        file = 'skills/audio-plugin-ux/references/fabfilter-pro-q-case-study.md'
        require = @(
            '\(multiple\)',
            'track/instance',
            'co-located measurement scales',
            'Two scales share one edge'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E5-user-laws-and-proof'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @(
            'No dead elements',
            'Material geometry is invariant across state',
            'getBoundingClientRect',
            'check-nakama-contract\.ps1',
            'Hold-to-audition ends'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E6-contract-script'
        file = 'skills/audio-plugin-ux/scripts/check-nakama-contract.ps1'
        require = @(
            'two-app-architecture',
            'one-graph-two-traces',
            'sidechain-closed-hidden',
            'material-geometry-law'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E7-current-visual-authority'
        file = 'skills/audio-plugin-ux/references/nakama-transfer.md'
        require = @(
            'design/abnahmen/2026-08-25-rework-referenz-drei-designs\.md',
            'design/assets/rework-basis-2026-08-25/gen-page-1-overview\.png',
            'design/assets/rework-basis-2026-08-25/gen-page-2-eq-center\.png',
            'design/assets/rework-basis-2026-08-25/probeeq\.png',
            'newest explicit user visual decision'
        )
        reject = @(
            'current Figma file named in'
        )
    },
    [pscustomobject]@{
        id = 'E8-visual-contract-check'
        file = 'skills/audio-plugin-ux/scripts/check-nakama-contract.ps1'
        require = @(
            'current-visual-rework-basis',
            'visual-router-updated',
            'gen-page-1-overview\.png',
            'gen-page-2-eq-center\.png',
            'probeeq\.png'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E9-access-response-contract'
        file = 'skills/audio-plugin-ux/SKILL.md'
        require = @(
            'Access and response contract',
            'pointer, keyboard, fine-adjustment, and text-entry',
            'name, role, value, state, and actions',
            'worst visual load',
            'noncritical pop-ups'
        )
        reject = @()
    },
    [pscustomobject]@{
        id = 'E10-access-response-tests'
        file = 'skills/audio-plugin-ux/references/design-reasoning.md'
        require = @(
            'Input-equivalence test',
            'Response-under-load test',
            'Interruption test',
            '24 CSS pixels',
            'Voger Design',
            'AccessibilityHandler'
        )
        reject = @(
            'every control must be 24'
        )
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

$report = [pscustomobject]@{
    baseline = Invoke-Suite -Label 'baseline-snapshot-0.2.1' -Root $BaselineRoot
    candidate = Invoke-Suite -Label 'source-candidate' -Root $CandidateRoot
}

$report | ConvertTo-Json -Depth 6
if ($report.candidate.passed -ne $report.candidate.total) { exit 1 }
exit 0
