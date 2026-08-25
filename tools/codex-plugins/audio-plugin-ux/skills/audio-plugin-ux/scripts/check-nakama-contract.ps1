#Requires -Version 7.0
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $RepoRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$checks = [System.Collections.Generic.List[object]]::new()

function Add-ContractCheck {
    param(
        [string] $Name,
        [bool] $Passed,
        [string] $Path,
        [string] $Detail
    )

    $checks.Add([pscustomobject]@{
        name   = $Name
        passed = $Passed
        path   = $Path
        detail = $Detail
    })
}

try {
    $root = (Resolve-Path -LiteralPath $RepoRoot -ErrorAction Stop).Path
}
catch {
    [pscustomobject]@{
        ok                    = $false
        pluginContractVersion = '0.2.1'
        repoRoot              = $RepoRoot
        checks                = @()
        error                 = "Repository root could not be resolved: $($_.Exception.Message)"
    } | ConvertTo-Json -Depth 6
    exit 2
}

function Test-ContractAnchor {
    param(
        [string] $Name,
        [string] $RelativePath,
        [string] $Pattern,
        [string] $Expected
    )

    $fullPath = Join-Path $root $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        Add-ContractCheck -Name $Name -Passed $false -Path $RelativePath -Detail 'Required file is missing.'
        return
    }

    $content = [System.IO.File]::ReadAllText($fullPath)
    $matches = [regex]::IsMatch(
        $content,
        $Pattern,
        [System.Text.RegularExpressions.RegexOptions]::IgnoreCase -bor
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )
    $detail = if ($matches) { $Expected } else { "Missing contract anchor: $Expected" }
    Add-ContractCheck -Name $Name -Passed $matches -Path $RelativePath -Detail $detail
}

$requiredFiles = @(
    'CLAUDE.md',
    'design/abnahmen/2026-08-25-rework-referenz-drei-designs.md',
    'design/assets/rework-basis-2026-08-25/gen-page-1-overview.png',
    'design/assets/rework-basis-2026-08-25/gen-page-2-eq-center.png',
    'design/assets/rework-basis-2026-08-25/probeeq.png',
    'design/abnahmen/2026-08-23-gen-eq-zentrale.md',
    'design/docs/ui-spielregeln-eq-zentrale.md',
    'design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest.md',
    'design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md',
    'design/abnahmen/2026-08-24-offene-figma-entscheide.md',
    'docs/bauaufteilung-sonden.md',
    'design/LIES-MICH.md',
    'docs/PLAN-STAND.md',
    'docs/review-2026-08-25-audio-plugin-ux.md',
    'design/werkzeug/sonde-messung.html',
    'tools/beweise.ps1'
)

foreach ($relativePath in $requiredFiles) {
    $exists = Test-Path -LiteralPath (Join-Path $root $relativePath) -PathType Leaf
    Add-ContractCheck -Name "file:$relativePath" -Passed $exists -Path $relativePath -Detail $(
        if ($exists) { 'Required source route exists.' } else { 'Required source route is missing.' }
    )
}

Test-ContractAnchor -Name 'two-app-architecture' -RelativePath 'CLAUDE.md' `
    -Pattern 'zwei Apps[\s\S]*Suna ist in Probeeq aufgegangen' `
    -Expected 'Gen and Probeeq are the two apps; Suna is absorbed into Probeeq.'
Test-ContractAnchor -Name 'current-visual-rework-basis' -RelativePath 'design/abnahmen/2026-08-25-rework-referenz-drei-designs.md' `
    -Pattern 'aktuelle Rework-Referenz[\s\S]*gen-page-1-overview\.png[\s\S]*gen-page-2-eq-center\.png[\s\S]*probeeq\.png[\s\S]*Frühere Referenzen sind Verlauf' `
    -Expected 'The three supplied PNGs are the current visual truth in progress; previous Figma and Suna visuals are history.'
Test-ContractAnchor -Name 'visual-router-updated' -RelativePath 'design/LIES-MICH.md' `
    -Pattern 'aktuelle visuelle Wahrheit in progress[\s\S]*rework-basis-2026-08-25[\s\S]*Live-Stand in Nakama-Design[\s\S]*abgelöst' `
    -Expected 'The design router points to the three new rework images and marks the former Figma-only rule superseded.'
Test-ContractAnchor -Name 'gen-master-eq' -RelativePath 'CLAUDE.md' `
    -Pattern 'Seite 2[\s\S]*vollwertige[mn]? Master-EQ' `
    -Expected 'Gen page 2 is the EQ center and includes a full master EQ.'
Test-ContractAnchor -Name 'one-graph-two-traces' -RelativePath 'design/abnahmen/2026-08-23-gen-eq-zentrale.md' `
    -Pattern '2 EQ spuren[\s\S]*EINEM Graph' `
    -Expected 'Two EQ traces share one graph.'
Test-ContractAnchor -Name 'switcher-on-page-two' -RelativePath 'design/abnahmen/2026-08-23-gen-eq-zentrale.md' `
    -Pattern '2\. seite[\s\S]*sonden durchklicken[\s\S]*nicht durch die[\s>]*1\. seite' `
    -Expected 'Probe switching lives directly on page 2.'
Test-ContractAnchor -Name 'band-inventory' -RelativePath 'design/docs/ui-spielregeln-eq-zentrale.md' `
    -Pattern '8 Band-Slots[\s\S]*je 13 Parametern' `
    -Expected 'The current stored inventory is 8 band slots x 13 parameters.'
Test-ContractAnchor -Name 'sidechain-closed-hidden' -RelativePath 'design/docs/ui-spielregeln-eq-zentrale.md' `
    -Pattern 'sidechain_source[\s\S]*ersten Release[\s\S]*unsichtbar[\s\S]*U5 / NAK-33 geschlossen' `
    -Expected 'Sidechain source is stored, hidden in release one, and U5/NAK-33 is closed.'
Test-ContractAnchor -Name 'hold-to-audition' -RelativePath 'design/docs/ui-spielregeln-eq-zentrale.md' `
    -Pattern 'Halten = hören[\s\S]*HOLD TO\s+AUDITION' `
    -Expected 'Audition is a hold/release contract, separate from apply.'
Test-ContractAnchor -Name 'fixed-aspect-information-budget' -RelativePath 'design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest.md' `
    -Pattern 'Festes Seitenverhältnis bleibt[\s\S]*8 Bänder × 13 Parameter \+ 5 globale' `
    -Expected 'The current design constraint is a fixed-aspect 8 x 13 + 5 information field.'
Test-ContractAnchor -Name 'no-dead-elements' -RelativePath 'design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md' `
    -Pattern 'Keine toten Elemente[\s\S]*Jedes sichtbare Element[\s\S]*(Handgriff|Zustand)' `
    -Expected 'Every visible element performs an action or reports honest state.'
Test-ContractAnchor -Name 'material-geometry-law' -RelativePath 'docs/review-2026-08-25-audio-plugin-ux.md' `
    -Pattern 'button ist ein material[\s\S]*getBoundingClientRect' `
    -Expected 'Control material keeps dimensions stable and is verified by rectangles.'
Test-ContractAnchor -Name 'historical-measurement-specimen' -RelativePath 'design/werkzeug/sonde-messung.html' `
    -Pattern 'Arbeitsstand, kein Urteil[\s\S]*700×420[\s\S]*8 Slots × 12 Parameter' `
    -Expected 'The measurement bank is explicitly historical evidence, not the current product inventory.'
Test-ContractAnchor -Name 'canonical-proof-runner' -RelativePath 'tools/beweise.ps1' `
    -Pattern 'Nakama-Beweis-Runner[\s\S]*\[CmdletBinding\(\)\]' `
    -Expected 'The canonical repository proof runner is present.'

$failed = @($checks | Where-Object { -not $_.passed })
$result = [pscustomobject]@{
    ok                    = ($failed.Count -eq 0)
    pluginContractVersion = '0.2.1'
    repoRoot              = $root
    checkedAt             = (Get-Date).ToString('o')
    checks                = @($checks)
    failedCount           = $failed.Count
}

$result | ConvertTo-Json -Depth 6
if ($failed.Count -gt 0) { exit 1 }
exit 0
