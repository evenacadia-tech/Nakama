#Requires -Version 7.0
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $RepoRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$adapterContractVersion = '0.3.0'
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
        ok                     = $false
        adapterContractVersion = $adapterContractVersion
        repoRoot               = $RepoRoot
        checks                 = @()
        error                  = "Repository root could not be resolved: $($_.Exception.Message)"
    } | ConvertTo-Json -Depth 7
    exit 2
}

function Test-ContractAnchor {
    param(
        [string] $Name,
        [string] $RelativePath,
        [string] $Pattern,
        [string] $Expected,
        [bool] $Reject = $false
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
    $passed = if ($Reject) { -not $matches } else { $matches }
    $detail = if ($passed) {
        $Expected
    }
    elseif ($Reject) {
        "Forbidden duplicate or stale anchor found: $Expected"
    }
    else {
        "Missing contract anchor: $Expected"
    }
    Add-ContractCheck -Name $Name -Passed $passed -Path $RelativePath -Detail $detail
}

$requiredFiles = @(
    'CLAUDE.md',
    'DESIGN.md',
    'UX-CONTRACT.md',
    'design/LIES-MICH.md',
    'docs/PLAN-STAND.md',
    'tools/beweise.ps1',
    'wissen/LIES-MICH.md',
    'wissen/AGENTS.md',
    'wissen/INDEX.md',
    'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/SKILL.md',
    'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/design-reasoning.md',
    'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/research-evidence.md',
    'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/nakama-transfer.md'
)

foreach ($relativePath in $requiredFiles) {
    $exists = Test-Path -LiteralPath (Join-Path $root $relativePath) -PathType Leaf
    Add-ContractCheck -Name "file:$relativePath" -Passed $exists -Path $relativePath -Detail $(
        if ($exists) { 'Required source route exists.' } else { 'Required source route is missing.' }
    )
}

Test-ContractAnchor -Name 'skill-routes-indexed-research' `
    -RelativePath 'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/SKILL.md' `
    -Pattern 'indexed research archive[\s\S]*references/research-evidence\.md[\s\S]*do not copy its findings' `
    -Expected 'The skill routes to indexed evidence without copying archive findings.'
Test-ContractAnchor -Name 'research-router-is-dynamic' `
    -RelativePath 'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/research-evidence.md' `
    -Pattern 'wissen/LIES-MICH\.md[\s\S]*wissen/AGENTS\.md[\s\S]*wissen/INDEX\.md[\s\S]*Do not hard-code today''s dated entry filenames' `
    -Expected 'Research discovery uses the current archive index rather than dated skill routes.'
Test-ContractAnchor -Name 'research-authority-boundary' `
    -RelativePath 'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/research-evidence.md' `
    -Pattern 'Source claim[\s\S]*Current product contract[\s\S]*Transfer inference[\s\S]*Design decision' `
    -Expected 'Source evidence, current contract, inference, and decision remain separate.'
Test-ContractAnchor -Name 'nakama-routes-current-owners' `
    -RelativePath 'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/nakama-transfer.md' `
    -Pattern 'CLAUDE\.md[\s\S]*DESIGN\.md[\s\S]*UX-CONTRACT\.md[\s\S]*Current source and tests[\s\S]*wissen/INDEX\.md' `
    -Expected 'The Nakama adapter resolves current product, visual, behavior, code, and research owners.'
Test-ContractAnchor -Name 'nakama-does-not-duplicate-product-snapshot' `
    -RelativePath 'tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/nakama-transfer.md' `
    -Pattern 'Current product contract to verify, not memorize' `
    -Expected 'No duplicated current-product snapshot remains in the adapter.' `
    -Reject $true
Test-ContractAnchor -Name 'archive-declares-skill-entrypoint' `
    -RelativePath 'wissen/LIES-MICH.md' `
    -Pattern 'INDEX\.md[\s\S]*(Skill|KI-Agent)' `
    -Expected 'The archive declares its index as the machine-consumable skill route.'

$knowledgeRoot = Join-Path $root 'wissen'
$indexPath = Join-Path $knowledgeRoot 'INDEX.md'
$indexContent = if (Test-Path -LiteralPath $indexPath -PathType Leaf) {
    [System.IO.File]::ReadAllText($indexPath)
}
else {
    ''
}

$entryRecords = [System.Collections.Generic.List[object]]::new()
if (Test-Path -LiteralPath $knowledgeRoot -PathType Container) {
    $markdownFiles = Get-ChildItem -LiteralPath $knowledgeRoot -Recurse -File -Filter '*.md'
    foreach ($file in $markdownFiles) {
        $relativePath = [System.IO.Path]::GetRelativePath($knowledgeRoot, $file.FullName).Replace('\', '/')
        if ($relativePath.StartsWith('vorlagen/', [System.StringComparison]::OrdinalIgnoreCase)) { continue }

        $content = [System.IO.File]::ReadAllText($file.FullName)
        $frontmatter = [regex]::Match($content, '\A---\s*\r?\n(?<body>.*?)\r?\n---', 'Singleline')
        if (-not $frontmatter.Success) { continue }

        $body = $frontmatter.Groups['body'].Value
        $typeMatch = [regex]::Match($body, '(?m)^typ:\s*(?<value>.+?)\s*$')
        if (-not $typeMatch.Success -or $typeMatch.Groups['value'].Value.Trim('"', "'") -ne 'wissenseintrag') { continue }

        $idMatch = [regex]::Match($body, '(?m)^id:\s*(?<value>.+?)\s*$')
        $statusMatch = [regex]::Match($body, '(?m)^status:\s*(?<value>.+?)\s*$')
        $verificationMatch = [regex]::Match($body, '(?m)^pruefstand:\s*(?<value>.+?)\s*$')
        $entryRecords.Add([pscustomobject]@{
            id = if ($idMatch.Success) { $idMatch.Groups['value'].Value.Trim('"', "'") } else { '' }
            status = if ($statusMatch.Success) { $statusMatch.Groups['value'].Value.Trim('"', "'") } else { '' }
            pruefstand = if ($verificationMatch.Success) { $verificationMatch.Groups['value'].Value.Trim('"', "'") } else { '' }
            relativePath = $relativePath
        })
    }
}

$missingMetadata = @($entryRecords | Where-Object { -not $_.id -or -not $_.status -or -not $_.pruefstand })
Add-ContractCheck -Name 'archive-entry-metadata' -Passed ($missingMetadata.Count -eq 0 -and $entryRecords.Count -gt 0) `
    -Path 'wissen/' -Detail $(
        if ($entryRecords.Count -eq 0) { 'No knowledge entries with typ: wissenseintrag were found.' }
        elseif ($missingMetadata.Count -gt 0) { "Missing id, status, or pruefstand: $($missingMetadata.relativePath -join ', ')" }
        else { "All $($entryRecords.Count) knowledge entries provide id, status, and pruefstand." }
    )

$duplicateIds = @($entryRecords | Where-Object id | Group-Object id | Where-Object Count -gt 1)
Add-ContractCheck -Name 'archive-unique-entry-ids' -Passed ($duplicateIds.Count -eq 0) -Path 'wissen/' -Detail $(
    if ($duplicateIds.Count -eq 0) { 'Knowledge entry IDs are unique.' }
    else { "Duplicate IDs: $($duplicateIds.Name -join ', ')" }
)

$allowedVerification = @('nur-extrakt', 'metadaten-geprueft', 'quellen-geprueft', 'mehrfach-belegt')
$invalidVerification = @($entryRecords | Where-Object { $_.status -eq 'aktiv' -and $_.pruefstand -notin $allowedVerification })
Add-ContractCheck -Name 'archive-allowed-verification-levels' -Passed ($invalidVerification.Count -eq 0) -Path 'wissen/' -Detail $(
    if ($invalidVerification.Count -eq 0) { 'Every active entry uses an allowed pruefstand value.' }
    else { "Invalid pruefstand: $($invalidVerification.relativePath -join ', ')" }
)

$activeEntries = @($entryRecords | Where-Object status -eq 'aktiv')
$indexFailures = [System.Collections.Generic.List[string]]::new()
foreach ($entry in $activeEntries) {
    $pattern = '\]\((?:\./)?' + [regex]::Escape($entry.relativePath) + '(?:#[^)]*)?\)'
    $count = [regex]::Matches($indexContent, $pattern, 'IgnoreCase').Count
    if ($count -ne 1) { $indexFailures.Add("$($entry.relativePath)=$count") }
}
Add-ContractCheck -Name 'archive-active-entries-indexed-once' -Passed ($indexFailures.Count -eq 0 -and $activeEntries.Count -gt 0) `
    -Path 'wissen/INDEX.md' -Detail $(
        if ($activeEntries.Count -eq 0) { 'No active knowledge entries were found.' }
        elseif ($indexFailures.Count -gt 0) { "Active entry link counts must equal one: $($indexFailures -join ', ')" }
        else { "All $($activeEntries.Count) active entries are indexed exactly once." }
    )

$brokenLinks = [System.Collections.Generic.List[string]]::new()
if ($indexContent) {
    $links = [regex]::Matches($indexContent, '\[[^\]]+\]\((?<target>[^)]+)\)')
    foreach ($link in $links) {
        $target = $link.Groups['target'].Value.Trim().Trim('<', '>')
        if ($target -match '^(?:[a-z]+:|#)') { continue }
        $target = ($target -replace '#.*$', '')
        if (-not $target) { continue }
        $decodedTarget = [uri]::UnescapeDataString($target)
        $resolvedTarget = [System.IO.Path]::GetFullPath((Join-Path $knowledgeRoot $decodedTarget))
        if (-not (Test-Path -LiteralPath $resolvedTarget)) { $brokenLinks.Add($target) }
    }
}
Add-ContractCheck -Name 'archive-index-relative-links-resolve' -Passed ($brokenLinks.Count -eq 0) -Path 'wissen/INDEX.md' -Detail $(
    if ($brokenLinks.Count -eq 0) { 'Every relative link in the knowledge index resolves.' }
    else { "Broken relative links: $($brokenLinks -join ', ')" }
)

$rawExtensions = @('.pdf', '.epub', '.txt', '.vtt', '.srt')
$rawSources = @(
    if (Test-Path -LiteralPath $knowledgeRoot -PathType Container) {
        Get-ChildItem -LiteralPath $knowledgeRoot -Recurse -File |
            Where-Object { $_.Extension.ToLowerInvariant() -in $rawExtensions }
    }
)
Add-ContractCheck -Name 'archive-no-raw-source-files' -Passed ($rawSources.Count -eq 0) -Path 'wissen/' -Detail $(
    if ($rawSources.Count -eq 0) { 'No raw PDF, ebook, transcript, subtitle, or text source is stored in the archive.' }
    else { "Raw source files found: $($rawSources.FullName -join ', ')" }
)

$failed = @($checks | Where-Object { -not $_.passed })
$result = [pscustomobject]@{
    ok                     = ($failed.Count -eq 0)
    adapterContractVersion = $adapterContractVersion
    repoRoot               = $root
    checkedAt              = (Get-Date).ToString('o')
    entryCount             = $entryRecords.Count
    activeEntryCount       = $activeEntries.Count
    checks                 = @($checks)
    failedCount            = $failed.Count
}

$result | ConvertTo-Json -Depth 7
if ($failed.Count -gt 0) { exit 1 }
exit 0
