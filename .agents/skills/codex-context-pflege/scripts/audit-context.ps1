[CmdletBinding()]
param(
    [Parameter()]
    [string] $WorkingDirectory = (Get-Location).Path,

    [Parameter()]
    [ValidateSet('Text', 'Json')]
    [string] $Format = 'Text',

    [Parameter()]
    [string] $CodexHome
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-Utf8ByteCount {
    param([Parameter(Mandatory)][string] $Text)

    return [Text.Encoding]::UTF8.GetByteCount($Text)
}

function Get-NonEmptyInstructionFile {
    param(
        [Parameter(Mandatory)][string] $Directory,
        [Parameter()][AllowEmptyCollection()][string[]] $FallbackNames = @()
    )

    $candidateNames = @('AGENTS.override.md', 'AGENTS.md') + $FallbackNames
    foreach ($name in $candidateNames) {
        $path = Join-Path $Directory $name
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            continue
        }

        $content = Get-Content -LiteralPath $path -Raw
        if (-not [string]::IsNullOrWhiteSpace($content)) {
            return [pscustomobject]@{
                Path = [IO.Path]::GetFullPath($path)
                Content = $content
            }
        }
    }

    return $null
}

function ConvertFrom-SimpleTomlValue {
    param([Parameter(Mandatory)][string] $Value)

    $trimmed = ($Value -replace '\s+#.*$', '').Trim()
    if ($trimmed -eq 'true') { return $true }
    if ($trimmed -eq 'false') { return $false }
    if ($trimmed -match '^[+-]?\d+$') { return [long]$trimmed }
    if ($trimmed.StartsWith('[') -and $trimmed.EndsWith(']')) {
        $matches = [regex]::Matches($trimmed, '["'']([^"'']*)["'']')
        return @($matches | ForEach-Object { $_.Groups[1].Value })
    }
    if (($trimmed.StartsWith('"') -and $trimmed.EndsWith('"')) -or
        ($trimmed.StartsWith("'") -and $trimmed.EndsWith("'"))) {
        return $trimmed.Substring(1, $trimmed.Length - 2)
    }

    return $trimmed
}

function Read-CodexContextConfig {
    param([Parameter(Mandatory)][string] $Path)

    $values = @{}
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $values
    }

    $section = ''
    foreach ($line in Get-Content -LiteralPath $Path) {
        $trimmed = $line.Trim()
        if ([string]::IsNullOrWhiteSpace($trimmed) -or $trimmed.StartsWith('#')) {
            continue
        }
        if ($trimmed -match '^\[([^\]]+)\]$') {
            $section = $Matches[1]
            continue
        }
        if ($trimmed -notmatch '^([A-Za-z0-9_.-]+)\s*=\s*(.+)$') {
            continue
        }

        $key = if ($section) { "$section.$($Matches[1])" } else { $Matches[1] }
        $values[$key] = ConvertFrom-SimpleTomlValue -Value $Matches[2]
    }

    return $values
}

function Get-ConfigValue {
    param(
        [Parameter(Mandatory)][hashtable] $Values,
        [Parameter(Mandatory)][string] $Key,
        [Parameter()] $Default
    )

    if ($Values.ContainsKey($Key)) {
        return $Values[$Key]
    }
    return $Default
}

function Get-ProjectDirectories {
    param(
        [Parameter(Mandatory)][string] $Root,
        [Parameter(Mandatory)][string] $Current
    )

    $rootPath = [IO.Path]::GetFullPath($Root).TrimEnd('\', '/')
    $currentPath = [IO.Path]::GetFullPath($Current).TrimEnd('\', '/')
    $relative = [IO.Path]::GetRelativePath($rootPath, $currentPath)
    if ($relative -eq '.') {
        return @($rootPath)
    }
    if ($relative -eq '..' -or $relative.StartsWith("..$([IO.Path]::DirectorySeparatorChar)")) {
        return @($currentPath)
    }

    $directories = [Collections.Generic.List[string]]::new()
    $directories.Add($rootPath)
    $cursor = $rootPath
    foreach ($segment in $relative.Split([IO.Path]::DirectorySeparatorChar, [StringSplitOptions]::RemoveEmptyEntries)) {
        $cursor = Join-Path $cursor $segment
        $directories.Add($cursor)
    }
    return @($directories)
}

function Remove-OuterYamlQuotes {
    param([Parameter(Mandatory)][string] $Value)

    $trimmed = $Value.Trim()
    if ($trimmed.Length -ge 2) {
        $first = $trimmed[0]
        $last = $trimmed[$trimmed.Length - 1]
        if (($first -eq '"' -and $last -eq '"') -or ($first -eq "'" -and $last -eq "'")) {
            return $trimmed.Substring(1, $trimmed.Length - 2)
        }
    }
    return $trimmed
}

function Get-SkillMetadata {
    param(
        [Parameter(Mandatory)][string] $Root,
        [Parameter(Mandatory)][string] $Scope
    )

    if (-not (Test-Path -LiteralPath $Root -PathType Container)) {
        return @()
    }

    $skills = [Collections.Generic.List[object]]::new()
    foreach ($skillFile in Get-ChildItem -LiteralPath $Root -Filter 'SKILL.md' -File -Recurse -Force) {
        $content = Get-Content -LiteralPath $skillFile.FullName -Raw
        $frontmatter = $null
        if ($content -match '(?ms)\A---\s*\r?\n(.*?)\r?\n---') {
            $frontmatter = $Matches[1]
        }

        $name = $null
        $description = $null
        if ($null -ne $frontmatter -and $frontmatter -match '(?m)^name:\s*(.*?)\s*$') {
            $name = Remove-OuterYamlQuotes -Value $Matches[1]
        }
        if ($null -ne $frontmatter -and $frontmatter -match '(?m)^description:\s*(.*?)\s*$') {
            $description = Remove-OuterYamlQuotes -Value $Matches[1]
        }

        $metadataCharacters = $skillFile.FullName.Length
        if ($null -ne $name) { $metadataCharacters += $name.Length }
        if ($null -ne $description) { $metadataCharacters += $description.Length }

        $skills.Add([pscustomobject]@{
            Scope = $Scope
            Name = $name
            Description = $description
            Path = $skillFile.FullName
            MetadataCharactersEstimate = $metadataCharacters
        })
    }

    return @($skills)
}

$resolvedWorkingDirectory = (Resolve-Path -LiteralPath $WorkingDirectory).Path
if ([string]::IsNullOrWhiteSpace($CodexHome)) {
    if (-not [string]::IsNullOrWhiteSpace($env:CODEX_HOME)) {
        $CodexHome = $env:CODEX_HOME
    } else {
        $CodexHome = Join-Path ([Environment]::GetFolderPath('UserProfile')) '.codex'
    }
}
$resolvedCodexHome = [IO.Path]::GetFullPath($CodexHome)
$configPath = Join-Path $resolvedCodexHome 'config.toml'
$config = Read-CodexContextConfig -Path $configPath

$instructionLimit = [long](Get-ConfigValue -Values $config -Key 'project_doc_max_bytes' -Default 32768)
$fallbackNames = @(Get-ConfigValue -Values $config -Key 'project_doc_fallback_filenames' -Default @())

$gitRootOutput = & git -C $resolvedWorkingDirectory rev-parse --show-toplevel 2>$null
if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($gitRootOutput)) {
    $projectRoot = [IO.Path]::GetFullPath(($gitRootOutput | Select-Object -First 1).Trim())
} else {
    $projectRoot = $resolvedWorkingDirectory
}

$instructionFiles = [Collections.Generic.List[object]]::new()
$globalInstruction = Get-NonEmptyInstructionFile -Directory $resolvedCodexHome -FallbackNames @()
if ($null -ne $globalInstruction) {
    $instructionFiles.Add([pscustomobject]@{
        Scope = 'global'
        Path = $globalInstruction.Path
        Bytes = Get-Utf8ByteCount -Text $globalInstruction.Content
    })
}

$projectDirectories = @(Get-ProjectDirectories -Root $projectRoot -Current $resolvedWorkingDirectory)
foreach ($directory in $projectDirectories) {
    $instruction = Get-NonEmptyInstructionFile -Directory $directory -FallbackNames $fallbackNames
    if ($null -eq $instruction) {
        continue
    }
    $instructionFiles.Add([pscustomobject]@{
        Scope = 'project'
        Path = $instruction.Path
        Bytes = Get-Utf8ByteCount -Text $instruction.Content
    })
}

$combinedInstructionBytes = 0L
foreach ($item in $instructionFiles) {
    $combinedInstructionBytes += [long]$item.Bytes
}
if ($instructionFiles.Count -gt 1) {
    $combinedInstructionBytes += 2L * ($instructionFiles.Count - 1)
}
$instructionUtilization = if ($instructionLimit -gt 0) {
    [Math]::Round(($combinedInstructionBytes / $instructionLimit) * 100, 1)
} else {
    0
}

$memoryDirectory = Join-Path $resolvedCodexHome 'memories'
$memoryFiles = @()
if (Test-Path -LiteralPath $memoryDirectory -PathType Container) {
    $memoryFiles = @(Get-ChildItem -LiteralPath $memoryDirectory -File -Recurse -Force |
        Where-Object { $_.FullName -notmatch '[\\/]\.git([\\/]|$)' })
}
$memoryBytes = if ($memoryFiles.Count -eq 0) {
    0L
} else {
    [long](($memoryFiles | Measure-Object -Property Length -Sum).Sum)
}
$coreMemoryFiles = @('memory_summary.md', 'MEMORY.md', 'raw_memories.md') | ForEach-Object {
    $path = Join-Path $memoryDirectory $_
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $item = Get-Item -LiteralPath $path
        [pscustomobject]@{
            Name = $_
            Exists = $true
            Bytes = [long]$item.Length
            LastWriteTimeUtc = $item.LastWriteTimeUtc.ToString('o')
        }
    } else {
        [pscustomobject]@{
            Name = $_
            Exists = $false
            Bytes = 0
            LastWriteTimeUtc = $null
        }
    }
}
$notesDirectory = Join-Path $memoryDirectory 'extensions\ad_hoc\notes'
$noteCount = if (Test-Path -LiteralPath $notesDirectory -PathType Container) {
    @(Get-ChildItem -LiteralPath $notesDirectory -File -Force).Count
} else {
    0
}

$memoriesFeature = Get-ConfigValue -Values $config -Key 'features.memories' -Default $false
$generateMemories = Get-ConfigValue -Values $config -Key 'memories.generate_memories' -Default $null
$useMemories = Get-ConfigValue -Values $config -Key 'memories.use_memories' -Default $null

$skillRoots = [Collections.Generic.List[object]]::new()
$skillRootPaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$userSkillRoot = Join-Path ([Environment]::GetFolderPath('UserProfile')) '.agents\skills'
if ($skillRootPaths.Add([IO.Path]::GetFullPath($userSkillRoot))) {
    $skillRoots.Add([pscustomobject]@{ Scope = 'user'; Path = [IO.Path]::GetFullPath($userSkillRoot) })
}
foreach ($directory in $projectDirectories) {
    $repoSkillRoot = Join-Path $directory '.agents\skills'
    $fullSkillRoot = [IO.Path]::GetFullPath($repoSkillRoot)
    if ($skillRootPaths.Add($fullSkillRoot)) {
        $skillRoots.Add([pscustomobject]@{ Scope = 'project'; Path = $fullSkillRoot })
    }
}

$skills = [Collections.Generic.List[object]]::new()
foreach ($root in $skillRoots) {
    foreach ($skill in Get-SkillMetadata -Root $root.Path -Scope $root.Scope) {
        $skills.Add($skill)
    }
}
$skillMetadataCharacters = 0L
foreach ($skill in $skills) {
    $skillMetadataCharacters += [long]$skill.MetadataCharactersEstimate
}
$skillMetadataBudget = 8000L
$skillMetadataUtilization = [Math]::Round(($skillMetadataCharacters / $skillMetadataBudget) * 100, 1)
$duplicateSkillNames = @($skills |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_.Name) } |
    Group-Object -Property Name |
    Where-Object Count -gt 1 |
    ForEach-Object {
        [pscustomobject]@{
            Name = $_.Name
            Paths = @($_.Group | ForEach-Object Path)
        }
    })
$invalidSkillMetadata = @($skills | Where-Object {
    [string]::IsNullOrWhiteSpace($_.Name) -or [string]::IsNullOrWhiteSpace($_.Description)
})

$findings = [Collections.Generic.List[object]]::new()
if ($instructionFiles.Count -eq 0) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'no-instructions'; Message = 'No non-empty AGENTS instruction file was discovered.' })
}
if ($combinedInstructionBytes -gt $instructionLimit) {
    $findings.Add([pscustomobject]@{ Severity = 'error'; Code = 'instruction-budget-exceeded'; Message = "Instruction chain uses $combinedInstructionBytes bytes, above the $instructionLimit-byte limit." })
} elseif ($instructionUtilization -ge 80) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'instruction-budget-high'; Message = "Instruction chain uses $instructionUtilization% of the configured limit." })
}
if (-not $memoriesFeature) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'memories-disabled'; Message = 'The Codex memories feature is disabled or not configured.' })
}
if ($memoriesFeature -and $generateMemories -eq $false) {
    $findings.Add([pscustomobject]@{ Severity = 'info'; Code = 'memory-generation-disabled'; Message = 'Existing memories may be used, but new chats are not configured for memory generation.' })
}
if ($memoriesFeature -and $useMemories -eq $false) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'memory-use-disabled'; Message = 'Existing memories are not configured to be injected into future sessions.' })
}
if (-not (Test-Path -LiteralPath $memoryDirectory -PathType Container)) {
    $findings.Add([pscustomobject]@{ Severity = 'info'; Code = 'memory-directory-missing'; Message = 'The local memory directory does not exist yet.' })
}
if ($skillMetadataCharacters -gt $skillMetadataBudget) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'local-skill-metadata-high'; Message = "Local user and repository skill metadata is estimated at $skillMetadataCharacters characters, above the $skillMetadataBudget-character host fallback budget." })
} elseif ($skillMetadataUtilization -ge 80) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'local-skill-metadata-near-budget'; Message = "Local user and repository skill metadata is estimated at $skillMetadataUtilization% of the $skillMetadataBudget-character host fallback budget." })
}
foreach ($duplicate in $duplicateSkillNames) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'duplicate-local-skill-name'; Message = "Local skill name '$($duplicate.Name)' appears at multiple paths." })
}
foreach ($invalid in $invalidSkillMetadata) {
    $findings.Add([pscustomobject]@{ Severity = 'warning'; Code = 'invalid-local-skill-metadata'; Message = "Local skill metadata is incomplete: $($invalid.Path)" })
}

$report = [ordered]@{
    SchemaVersion = 1
    WorkingDirectory = $resolvedWorkingDirectory
    ProjectRoot = $projectRoot
    CodexHome = $resolvedCodexHome
    ConfigPath = $configPath
    Instructions = [ordered]@{
        LimitBytes = $instructionLimit
        CombinedBytes = $combinedInstructionBytes
        UtilizationPercent = $instructionUtilization
        Files = @($instructionFiles)
    }
    Memories = [ordered]@{
        FeatureEnabled = $memoriesFeature
        GenerateMemories = $generateMemories
        UseMemories = $useMemories
        Directory = $memoryDirectory
        FileCount = $memoryFiles.Count
        TotalBytes = $memoryBytes
        CoreFiles = @($coreMemoryFiles)
        AdHocNotesDirectory = $notesDirectory
        AdHocNotesDirectoryExists = (Test-Path -LiteralPath $notesDirectory -PathType Container)
        AdHocNoteCount = $noteCount
    }
    Skills = [ordered]@{
        Coverage = 'local user and repository skills only; host system and plugin skills excluded'
        MetadataFallbackBudgetCharacters = $skillMetadataBudget
        MetadataCharactersEstimate = $skillMetadataCharacters
        MetadataUtilizationPercent = $skillMetadataUtilization
        Count = $skills.Count
        Roots = @($skillRoots)
        Files = @($skills)
        DuplicateNames = @($duplicateSkillNames)
    }
    Findings = @($findings)
}

if ($Format -eq 'Json') {
    $report | ConvertTo-Json -Depth 8
    return
}

Write-Output 'Codex context audit'
Write-Output "Working directory: $resolvedWorkingDirectory"
Write-Output "Instruction budget: $combinedInstructionBytes / $instructionLimit bytes ($instructionUtilization%)"
foreach ($item in $instructionFiles) {
    Write-Output "  [$($item.Scope)] $($item.Path) ($($item.Bytes) bytes)"
}
Write-Output "Memories: feature=$memoriesFeature generate=$generateMemories use=$useMemories"
Write-Output "Memory store: $($memoryFiles.Count) files, $memoryBytes bytes"
Write-Output "Local skills: $($skills.Count), estimated metadata $skillMetadataCharacters / $skillMetadataBudget characters ($skillMetadataUtilization%)"
if ($findings.Count -eq 0) {
    Write-Output 'Findings: none'
} else {
    Write-Output 'Findings:'
    foreach ($finding in $findings) {
        Write-Output "  [$($finding.Severity)] $($finding.Code): $($finding.Message)"
    }
}
