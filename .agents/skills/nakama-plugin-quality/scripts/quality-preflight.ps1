[CmdletBinding()]
param(
    [Parameter()]
    [string] $WorkingDirectory = (Get-Location).Path,

    [Parameter()]
    [ValidateSet('Text', 'Json')]
    [string] $Format = 'Text',

    [Parameter()]
    [string] $BaseRef
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Invoke-GitLines {
    param(
        [Parameter(Mandatory)][string] $Root,
        [Parameter(Mandatory)][string[]] $Arguments,
        [switch] $AllowFailure
    )

    $previousErrorActionPreference = $ErrorActionPreference
    try {
        # Windows PowerShell 5.1 wraps native stderr as ErrorRecord objects.
        # AllowFailure must therefore survive stderr as well as a non-zero exit.
        $ErrorActionPreference = 'Continue'
        $output = @(& git -C $Root @Arguments 2>$null)
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($exitCode -ne 0 -and -not $AllowFailure) {
        throw "git $($Arguments -join ' ') failed with exit code $exitCode"
    }
    return [pscustomobject]@{
        ExitCode = $exitCode
        Lines = @($output | ForEach-Object { ([string]$_).Trim() } | Where-Object { $_ })
    }
}

function Get-CommandInfo {
    param(
        [Parameter(Mandatory)][string] $Name,
        [Parameter(Mandatory)][bool] $Required
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    return [pscustomobject]@{
        Name = $Name
        Required = $Required
        Available = $null -ne $command
        Path = if ($null -eq $command) { $null } else { $command.Source }
    }
}

function Get-ScopeTag {
    param([Parameter(Mandatory)][string] $Path)

    $normalized = $Path.Replace('\', '/')
    switch -Regex ($normalized) {
        '^eq-copilot/plugin/' { return 'plugin' }
        '^eq-copilot/schemas/' { return 'contracts-state' }
        '^eq-copilot/fixtures/' { return 'contracts-state' }
        '^eq-copilot/install/' { return 'installer' }
        '^eq-copilot/identity/' { return 'identity-host' }
        '^broker/' { return 'broker' }
        '^design/' { return 'design' }
        '^wissen/' { return 'knowledge' }
        '^\.agents/skills/' { return 'quality-context-tooling' }
        '^tools/' { return 'tooling' }
        '^docs/beweise/' { return 'evidence' }
        '^docs/plan/' { return 'plan' }
        '^docs/PLAN-STAND\.md$' { return 'plan' }
        '^docs/FL-Nakama-Sonden-Design-Entwurf\.md$' { return 'plan' }
        '^docs/plugin-wissen\.md$' { return 'architecture-doc' }
        '^(AGENTS|CLAUDE)\.md$' { return 'quality-context-tooling' }
        default { return 'other' }
    }
}

$resolvedWorkingDirectory = (Resolve-Path -LiteralPath $WorkingDirectory).Path
$gitRoot = Invoke-GitLines -Root $resolvedWorkingDirectory -Arguments @('rev-parse', '--show-toplevel')
$projectRoot = [IO.Path]::GetFullPath($gitRoot.Lines[0])

$findings = [Collections.Generic.List[object]]::new()
$requiredPaths = [ordered]@{
    Instructions = 'AGENTS.md'
    ProductContext = 'CLAUDE.md'
    PlanStatus = 'docs/PLAN-STAND.md'
    ProbePlan = 'docs/FL-Nakama-Sonden-Design-Entwurf.md'
    Architecture = 'docs/plugin-wissen.md'
    EvidenceRunner = 'tools/beweise.ps1'
    KnowledgeIndex = 'wissen/INDEX.md'
    SkillEntrypoint = '.agents/skills/nakama-plugin-quality/SKILL.md'
    QualityModel = '.agents/skills/nakama-plugin-quality/references/quality-model.md'
}

$pathChecks = [Collections.Generic.List[object]]::new()
foreach ($entry in $requiredPaths.GetEnumerator()) {
    $fullPath = Join-Path $projectRoot $entry.Value
    $exists = Test-Path -LiteralPath $fullPath -PathType Leaf
    $pathChecks.Add([pscustomobject]@{
        Role = $entry.Key
        RelativePath = $entry.Value
        Exists = $exists
        LastWriteTimeUtc = if ($exists) { (Get-Item -LiteralPath $fullPath).LastWriteTimeUtc.ToString('o') } else { $null }
    })
    if (-not $exists) {
        $findings.Add([pscustomobject]@{
            Severity = 'error'
            Code = 'canonical-path-missing'
            Message = "Required $($entry.Key) file is missing: $($entry.Value)"
        })
    }
}

$planStatusPath = Join-Path $projectRoot 'docs/PLAN-STAND.md'
if (Test-Path -LiteralPath $planStatusPath -PathType Leaf) {
    $planStatus = Get-Content -LiteralPath $planStatusPath -Raw
    if ($planStatus -notmatch '<!--\s*quellstand:\s*[0-9a-f]+\s*-->') {
        $findings.Add([pscustomobject]@{
            Severity = 'error'
            Code = 'plan-status-not-generated'
            Message = 'docs/PLAN-STAND.md has no generated source marker.'
        })
    }
}

$tools = @(
    Get-CommandInfo -Name 'git' -Required $true
    Get-CommandInfo -Name 'py' -Required $true
    Get-CommandInfo -Name 'pwsh' -Required $true
    Get-CommandInfo -Name 'cargo' -Required $true
    Get-CommandInfo -Name 'python' -Required $false
    Get-CommandInfo -Name 'cmake' -Required $false
)
foreach ($tool in $tools) {
    if ($tool.Required -and -not $tool.Available) {
        $findings.Add([pscustomobject]@{
            Severity = 'warning'
            Code = "required-tool-missing-$($tool.Name)"
            Message = if ($tool.Name -eq 'pwsh') {
                'PowerShell 7 (pwsh) is unavailable; the canonical tools/beweise.ps1 runner cannot execute.'
            } else {
                "Required quality tool is unavailable: $($tool.Name)"
            }
        })
    }
}

$head = (Invoke-GitLines -Root $projectRoot -Arguments @('rev-parse', 'HEAD')).Lines[0]
$branchResult = Invoke-GitLines -Root $projectRoot -Arguments @('rev-parse', '--abbrev-ref', 'HEAD')
$branch = if ($branchResult.Lines.Count -eq 0) { '(detached)' } else { $branchResult.Lines[0] }
$upstreamResult = Invoke-GitLines -Root $projectRoot -Arguments @('rev-parse', '--abbrev-ref', '--symbolic-full-name', '@{upstream}') -AllowFailure
$upstream = if ($upstreamResult.ExitCode -eq 0 -and $upstreamResult.Lines.Count -gt 0) { $upstreamResult.Lines[0] } else { $null }
$ahead = $null
$behind = $null
if ($null -ne $upstream) {
    $counts = Invoke-GitLines -Root $projectRoot -Arguments @('rev-list', '--left-right', '--count', "HEAD...$upstream")
    if ($counts.Lines.Count -gt 0 -and $counts.Lines[0] -match '^(\d+)\s+(\d+)$') {
        $ahead = [int]$Matches[1]
        $behind = [int]$Matches[2]
    }
}

$changedPaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($arguments in @(
    @('diff', '--name-only', '--'),
    @('diff', '--cached', '--name-only', '--'),
    @('ls-files', '--others', '--exclude-standard')
)) {
    $result = Invoke-GitLines -Root $projectRoot -Arguments $arguments
    foreach ($line in $result.Lines) { [void]$changedPaths.Add($line.Replace('\', '/')) }
}

$baseCommit = $null
if (-not [string]::IsNullOrWhiteSpace($BaseRef)) {
    $verify = Invoke-GitLines -Root $projectRoot -Arguments @('rev-parse', '--verify', "$BaseRef^{commit}") -AllowFailure
    if ($verify.ExitCode -ne 0 -or $verify.Lines.Count -eq 0) {
        $findings.Add([pscustomobject]@{
            Severity = 'error'
            Code = 'invalid-base-ref'
            Message = "BaseRef does not resolve to a commit: $BaseRef"
        })
    } else {
        $baseCommit = $verify.Lines[0]
        $committedDiff = Invoke-GitLines -Root $projectRoot -Arguments @('diff', '--name-only', "$BaseRef...HEAD", '--')
        foreach ($line in $committedDiff.Lines) { [void]$changedPaths.Add($line.Replace('\', '/')) }
    }
}

$statusLines = (Invoke-GitLines -Root $projectRoot -Arguments @('status', '--short')).Lines
$scopeTags = @($changedPaths | ForEach-Object { Get-ScopeTag -Path $_ } | Sort-Object -Unique)
if ($changedPaths.Count -eq 0) {
    $findings.Add([pscustomobject]@{
        Severity = 'info'
        Code = 'no-local-scope'
        Message = 'No working-tree or selected BaseRef changes were found; derive scope from the explicit user request.'
    })
}

$contextReady = @($pathChecks | Where-Object { -not $_.Exists }).Count -eq 0 -and
    @($findings | Where-Object { $_.Severity -eq 'error' }).Count -eq 0
$missingRequiredTools = @($tools | Where-Object { $_.Required -and -not $_.Available })
$canonicalRunnerInvocable = $contextReady -and $missingRequiredTools.Count -eq 0

$report = [ordered]@{
    SchemaVersion = 1
    ProjectRoot = $projectRoot
    Revision = $head
    Branch = $branch
    Upstream = [ordered]@{
        Name = $upstream
        Ahead = $ahead
        Behind = $behind
        Note = 'Local tracking state only; preflight does not fetch.'
    }
    Scope = [ordered]@{
        BaseRef = $BaseRef
        BaseCommit = $baseCommit
        WorkingTreeDirty = $statusLines.Count -gt 0
        Status = @($statusLines)
        ChangedPaths = @($changedPaths | Sort-Object)
        Tags = $scopeTags
    }
    Canonical = [ordered]@{
        ContextReady = $contextReady
        CanonicalRunnerInvocable = $canonicalRunnerInvocable
        Paths = @($pathChecks)
    }
    Tools = $tools
    Findings = @($findings)
}

if ($Format -eq 'Json') {
    $report | ConvertTo-Json -Depth 8
    return
}

Write-Output 'Nakama plugin quality preflight'
Write-Output "Revision: $head ($branch)"
Write-Output "Scope tags: $($scopeTags -join ', ')"
Write-Output "Changed paths: $($changedPaths.Count)"
Write-Output "Context ready: $contextReady"
Write-Output "Canonical runner invocable: $canonicalRunnerInvocable"
if ($findings.Count -eq 0) {
    Write-Output 'Findings: none'
} else {
    Write-Output 'Findings:'
    foreach ($finding in $findings) {
        Write-Output "  [$($finding.Severity)] $($finding.Code): $($finding.Message)"
    }
}
