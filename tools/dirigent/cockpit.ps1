[CmdletBinding(DefaultParameterSetName = 'StatusLine')]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'StatusLine')]
    [switch]$StatusLine,

    [Parameter(Mandatory = $true, ParameterSetName = 'Plan')]
    [switch]$Plan,

    [Parameter(ParameterSetName = 'Plan')]
    [string]$CurrentStep = '',

    [Parameter(Mandatory = $true, ParameterSetName = 'WatchWorker')]
    [switch]$WatchWorker,

    [Parameter(Mandatory = $true, ParameterSetName = 'WatchWorker')]
    [ValidatePattern('^[A-Za-z0-9-]+$')]
    [string]$WorkerId,

    [Parameter(Mandatory = $true, ParameterSetName = 'WatchWorker')]
    [ValidatePattern('^[A-Fa-f0-9]{7,40}$')]
    [string]$BaseSha,

    [Parameter(ParameterSetName = 'WatchWorker')]
    [ValidateSet('LOCKER', 'NORMAL', 'ENG')]
    [string]$Aufsicht = 'NORMAL',

    [Parameter(ParameterSetName = 'WatchWorker')]
    [ValidateSet('Opus')]
    [string]$StartModel = 'Opus',

    [Parameter(ParameterSetName = 'WatchWorker')]
    [ValidateSet('max')]
    [string]$StartEffort = 'max',

    [Parameter(Mandatory = $true, ParameterSetName = 'WatchWorker')]
    [ValidatePattern('^[A-Za-z0-9-]+$')]
    [string]$DirigentSessionId,

    [Parameter(ParameterSetName = 'WatchWorker')]
    [ValidateRange(5, 60)]
    [int]$PollSeconds = 5,

    [Parameter(ParameterSetName = 'WatchWorker')]
    [ValidateRange(0, 86400)]
    [int]$TimeoutSeconds = 0
)

$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8

$script:RepoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$script:PlanJson = Join-Path $script:RepoRoot 'docs\plan\plan.json'
$script:PlanStand = Join-Path $script:RepoRoot 'docs\PLAN-STAND.md'
$script:FragenJson = Join-Path $script:RepoRoot 'docs\plan\fragen.json'

function Get-PropertyValue {
    param([object]$Object, [string]$Name)
    if ($null -eq $Object) { return $null }
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) { return $null }
    return $property.Value
}

function Get-NestedValue {
    param([object]$Object, [string[]]$Path)
    $value = $Object
    foreach ($part in $Path) {
        $value = Get-PropertyValue $value $part
        if ($null -eq $value) { return $null }
    }
    return $value
}

function Test-ProcessArgument {
    param([string]$CommandLine, [string]$Name, [string]$Value)
    if (-not $CommandLine -or -not $Name -or -not $Value) { return $false }
    $pattern = '(?i)(?:^|\s)-' + [regex]::Escape($Name) +
        '\s+["'']?' + [regex]::Escape($Value) + '["'']?(?:\s|$)'
    return [bool]($CommandLine -match $pattern)
}

function Get-UnixNow {
    return [DateTimeOffset]::UtcNow.ToUnixTimeSeconds()
}

function Get-CachePath {
    param([string]$SessionId, [string]$Kind)
    $safe = ($SessionId -replace '[^A-Za-z0-9-]', '_')
    if (-not $safe) { $safe = 'unknown' }
    return Join-Path ([IO.Path]::GetTempPath()) "nakama-dirigent-$safe-$Kind.json"
}

function Read-JsonFile {
    param([string]$Path)
    try {
        if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { return $null }
        return [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch {
        return $null
    }
}

function Write-JsonFileAtomic {
    param([string]$Path, [object]$Value)
    $temporary = "$Path.$PID.tmp"
    try {
        $json = $Value | ConvertTo-Json -Depth 20 -Compress
        [IO.File]::WriteAllText($temporary, $json, $utf8)
        Move-Item -LiteralPath $temporary -Destination $Path -Force
    }
    catch {
        if (Test-Path -LiteralPath $temporary) {
            Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
        }
    }
}

function Quote-ProcessArgument {
    param([string]$Value)
    return '"' + ($Value -replace '"', '\"') + '"'
}

function Invoke-TextProcess {
    param(
        [string]$FilePath,
        [string]$Arguments,
        [int]$TimeoutSeconds = 5
    )
    $process = $null
    try {
        $start = New-Object System.Diagnostics.ProcessStartInfo
        $start.FileName = $FilePath
        $start.Arguments = $Arguments
        $start.WorkingDirectory = $script:RepoRoot
        $start.UseShellExecute = $false
        $start.RedirectStandardOutput = $true
        $start.RedirectStandardError = $true
        $start.CreateNoWindow = $true
        $process = New-Object System.Diagnostics.Process
        $process.StartInfo = $start
        [void]$process.Start()
        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            $process.Kill()
            [void]$process.WaitForExit(1000)
            return [pscustomobject]@{ Ok = $false; ExitCode = -1; StdOut = ''; Error = 'Zeitlimit' }
        }
        [void]$stdoutTask.Wait(1000)
        [void]$stderrTask.Wait(1000)
        return [pscustomobject]@{
            Ok = ($process.ExitCode -eq 0)
            ExitCode = $process.ExitCode
            StdOut = $stdoutTask.Result
            Error = $stderrTask.Result.Trim()
        }
    }
    catch {
        return [pscustomobject]@{ Ok = $false; ExitCode = -1; StdOut = ''; Error = $_.Exception.Message }
    }
    finally {
        if ($null -ne $process) { $process.Dispose() }
    }
}

function Invoke-GitText {
    param([string]$Arguments, [int]$TimeoutSeconds = 4)
    try {
        $gitPath = (Get-Command git -ErrorAction Stop).Source
        return Invoke-TextProcess $gitPath "-C $(Quote-ProcessArgument $script:RepoRoot) $Arguments" $TimeoutSeconds
    }
    catch {
        return [pscustomobject]@{ Ok = $false; ExitCode = -1; StdOut = ''; Error = $_.Exception.Message }
    }
}

function Get-PlanSnapshot {
    $result = [ordered]@{
        Ok = $false
        Error = ''
        Accepted = 0
        Built = 0
        Total = 0
        Open = 0
        SourceSha = ''
        CurrentSourceSha = ''
        DirtySources = $false
        DirtyTree = $false
        WorktreeCount = 0
        Questions = 0
        QuestionsAvailable = $false
        QuestionText = ''
        Next = ''
        StatusById = @{}
        Plan = $null
    }
    try {
        $text = [IO.File]::ReadAllText($script:PlanStand, [Text.Encoding]::UTF8)
        $planData = [IO.File]::ReadAllText($script:PlanJson, [Text.Encoding]::UTF8) | ConvertFrom-Json
        if ($text -notmatch '\*\*(\d+) von (\d+) abgenommen\*\* · (\d+) gebaut · (\d+) offen') {
            throw 'Bilanzzeile in docs/PLAN-STAND.md ist unlesbar'
        }
        $result.Accepted = [int]$Matches[1]
        $result.Total = [int]$Matches[2]
        $result.Built = [int]$Matches[3]
        $result.Open = $result.Total - $result.Accepted
        if ($text -match '<!-- quellstand: ([0-9a-fA-F]+) -->') { $result.SourceSha = $Matches[1] }
        if ($text -match '\*\*Als Nächstes:\*\*\s*(.+)') { $result.Next = $Matches[1].Trim() }

        foreach ($line in ($text -split "`r?`n")) {
            if ($line -match '^- ([■▣□]) \*\*(.+?)\*\*.*\((abgenommen|gebaut|offen)(?: ·|\))') {
                $result.StatusById[$Matches[2]] = $Matches[3]
            }
        }
        $steps = @($planData.phasen | ForEach-Object { @($_.schritte) })
        if ($steps.Count -ne $result.Total -or $result.StatusById.Count -ne $result.Total) {
            throw "Paketmenge weicht ab (Plan $($steps.Count), Blatt $($result.StatusById.Count), Bilanz $($result.Total))"
        }
        foreach ($step in $steps) {
            $name = [string](Get-PropertyValue $step 'leitungsname')
            if (-not $name.Trim() -or $name -match "`r|`n" -or $name.Length -gt 90) {
                throw "Ungültiger leitungsname für $($step.id)"
            }
        }

        $source = Invoke-GitText 'log -1 --format=%h -- docs/plan docs/beweise tools/plan'
        if ($source.Ok) { $result.CurrentSourceSha = $source.StdOut.Trim() }
        $dirtySources = Invoke-GitText 'status --porcelain -- docs/plan docs/beweise tools/plan'
        if (-not $dirtySources.Ok) { throw 'Git-Status der Planquellen nicht lesbar' }
        $result.DirtySources = [bool]$dirtySources.StdOut.Trim()
        $dirtyTree = Invoke-GitText 'status --porcelain'
        if (-not $dirtyTree.Ok) { throw 'Git-Arbeitsbaum nicht lesbar' }
        $result.DirtyTree = [bool]$dirtyTree.StdOut.Trim()
        $worktrees = Invoke-GitText 'worktree list --porcelain'
        if (-not $worktrees.Ok) { throw 'Git-Worktrees nicht lesbar' }
        $result.WorktreeCount = @(($worktrees.StdOut -split "`r?`n") | Where-Object { $_ -match '^worktree ' }).Count
        $questions = [IO.File]::ReadAllText($script:FragenJson, [Text.Encoding]::UTF8) | ConvertFrom-Json
        $openQuestions = @($questions.offen)
        $result.Questions = $openQuestions.Count
        if ($openQuestions.Count) {
            $questionText = [string](Get-PropertyValue $openQuestions[0] 'titel')
            if (-not $questionText.Trim()) { throw 'Erste offene Frage besitzt keinen Titel' }
            $questionText = ($questionText -replace '[\r\n]+', ' ').Trim()
            if ($questionText.Length -gt 80) { $questionText = $questionText.Substring(0, 77).TrimEnd() + '…' }
            $result.QuestionText = $questionText
        }
        $result.QuestionsAvailable = $true
        $result.Plan = $planData
        $result.Ok = [bool]$result.SourceSha -and [bool]$result.CurrentSourceSha
        if (-not $result.Ok) { $result.Error = 'Git- oder Quellstand nicht lesbar' }
    }
    catch {
        $result.Error = $_.Exception.Message
    }
    return [pscustomobject]$result
}

function Get-AgentSnapshot {
    param([string]$SessionId, [switch]$NoCache)
    $cachePath = Get-CachePath $SessionId 'agents'
    $cached = if ($NoCache) { $null } else { Read-JsonFile $cachePath }
    $now = Get-UnixNow
    $cachedAt = Get-PropertyValue $cached 'At'
    if ($null -ne $cachedAt -and ($now - [long]$cachedAt) -ge 0 -and ($now - [long]$cachedAt) -lt 5) {
        return $cached
    }
    try {
        $claude = (Get-Command claude -ErrorAction Stop).Source
        $call = Invoke-TextProcess $claude "agents --json --cwd $(Quote-ProcessArgument $script:RepoRoot) --all" 5
        if (-not $call.Ok) { throw ($call.Error | Out-String) }
        $agents = @($call.StdOut | ConvertFrom-Json)
        $snapshot = [pscustomobject]@{ At = $now; Ok = $true; Error = ''; Agents = $agents }
    }
    catch {
        $snapshot = [pscustomobject]@{ At = $now; Ok = $false; Error = $_.Exception.Message; Agents = @() }
    }
    if (-not $NoCache) { Write-JsonFileAtomic $cachePath $snapshot }
    return $snapshot
}

function Get-ClaudeRateSnapshot {
    param([object]$InputData, [string]$SessionId)
    $cachePath = Get-CachePath $SessionId 'claude'
    $cached = Read-JsonFile $cachePath
    $now = Get-UnixNow
    $five = Get-NestedValue $InputData @('rate_limits', 'five_hour')
    $seven = Get-NestedValue $InputData @('rate_limits', 'seven_day')
    $currentAvailable = ($null -ne $five -and $null -ne $seven)
    $cachedAt = Get-PropertyValue $cached 'At'
    $cachedAvailable = [bool](Get-PropertyValue $cached 'Available')
    if ($null -ne $cachedAt -and ($now - [long]$cachedAt) -ge 0 -and
        ($now - [long]$cachedAt) -lt 60 -and ($cachedAvailable -or -not $currentAvailable)) {
        return $cached
    }
    $snapshot = [pscustomobject]@{
        At = $now
        Available = $currentAvailable
        FiveUsed = Get-PropertyValue $five 'used_percentage'
        FiveReset = Get-PropertyValue $five 'resets_at'
        SevenUsed = Get-PropertyValue $seven 'used_percentage'
        SevenReset = Get-PropertyValue $seven 'resets_at'
    }
    Write-JsonFileAtomic $cachePath $snapshot
    return $snapshot
}

function Invoke-CodexRateLimits {
    $process = $null
    try {
        $codex = Get-Command codex -ErrorAction Stop
        $start = New-Object System.Diagnostics.ProcessStartInfo
        if ($codex.Source -match '\.ps1$') {
            $start.FileName = (Get-Process -Id $PID).Path
            $start.Arguments = "-NoProfile -File $(Quote-ProcessArgument $codex.Source) app-server --stdio"
        }
        else {
            $start.FileName = $codex.Source
            $start.Arguments = 'app-server --stdio'
        }
        $start.WorkingDirectory = $script:RepoRoot
        $start.UseShellExecute = $false
        $start.RedirectStandardInput = $true
        $start.RedirectStandardOutput = $true
        $start.RedirectStandardError = $true
        $start.CreateNoWindow = $true
        $process = New-Object System.Diagnostics.Process
        $process.StartInfo = $start
        [void]$process.Start()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        $process.StandardInput.WriteLine('{"method":"initialize","id":1,"params":{"clientInfo":{"name":"nakama_cockpit","title":"Nakama Cockpit","version":"1.0.0"}}}')
        $process.StandardInput.WriteLine('{"method":"initialized","params":{}}')
        $process.StandardInput.WriteLine('{"method":"account/rateLimits/read","id":2}')
        $process.StandardInput.Flush()

        $deadline = [DateTime]::UtcNow.AddSeconds(6)
        $response = $null
        while ([DateTime]::UtcNow -lt $deadline) {
            $lineTask = $process.StandardOutput.ReadLineAsync()
            if (-not $lineTask.Wait(3000)) { break }
            $line = $lineTask.Result
            if ($null -eq $line) { break }
            try {
                $message = $line | ConvertFrom-Json
                if ((Get-PropertyValue $message 'id') -eq 2) {
                    $response = $message
                    break
                }
            }
            catch { }
        }
        if ($null -eq $response) { throw 'account/rateLimits/read ohne Antwort' }
        $errorValue = Get-PropertyValue $response 'error'
        if ($null -ne $errorValue) { throw ([string](Get-PropertyValue $errorValue 'message')) }
        $result = Get-PropertyValue $response 'result'
        $bucket = Get-NestedValue $result @('rateLimitsByLimitId', 'codex')
        if ($null -eq $bucket) { $bucket = Get-PropertyValue $result 'rateLimits' }
        $weekly = $null
        foreach ($candidate in @((Get-PropertyValue $bucket 'primary'), (Get-PropertyValue $bucket 'secondary'))) {
            if ($null -ne $candidate -and [int](Get-PropertyValue $candidate 'windowDurationMins') -eq 10080) {
                $weekly = $candidate
                break
            }
        }
        if ($null -eq $weekly) { throw 'kein ausdrücklich gemeldetes 7-Tage-Fenster' }
        return [pscustomobject]@{
            Available = $true
            Used = Get-PropertyValue $weekly 'usedPercent'
            Reset = Get-PropertyValue $weekly 'resetsAt'
            WindowMinutes = Get-PropertyValue $weekly 'windowDurationMins'
            Error = ''
        }
    }
    catch {
        return [pscustomobject]@{ Available = $false; Used = $null; Reset = $null; WindowMinutes = $null; Error = $_.Exception.Message }
    }
    finally {
        if ($null -ne $process) {
            try {
                if (-not $process.HasExited) { $process.Kill() }
                [void]$process.WaitForExit(1000)
            }
            catch { }
            $process.Dispose()
        }
    }
}

function Get-CodexRateSnapshot {
    param([string]$SessionId)
    $cachePath = Get-CachePath $SessionId 'codex'
    $cached = Read-JsonFile $cachePath
    $now = Get-UnixNow
    $cachedAt = Get-PropertyValue $cached 'At'
    if ($null -ne $cachedAt -and ($now - [long]$cachedAt) -ge 0 -and ($now - [long]$cachedAt) -lt 60) {
        return $cached
    }
    $read = Invoke-CodexRateLimits
    $snapshot = [pscustomobject]@{
        At = $now
        Available = [bool]$read.Available
        Used = $read.Used
        Reset = $read.Reset
        WindowMinutes = $read.WindowMinutes
        Error = $read.Error
    }
    Write-JsonFileAtomic $cachePath $snapshot
    return $snapshot
}

function Get-TranscriptEvidence {
    param([string]$TranscriptPath, [string]$SessionId, [string]$HeadSha)
    $empty = [pscustomobject]@{ PlanRead = $false; HeadRead = $false; QuestionWaiting = $false; WorkerContracts = @(); CodexContracts = @(); ControlMinutes = 0; ReadPaths = @() }
    if (-not $TranscriptPath -or -not (Test-Path -LiteralPath $TranscriptPath -PathType Leaf)) { return $empty }
    $file = Get-Item -LiteralPath $TranscriptPath
    $cachePath = Get-CachePath $SessionId 'transcript'
    $cached = Read-JsonFile $cachePath
    if ($null -ne $cached -and (Get-PropertyValue $cached 'Length') -eq $file.Length -and
        (Get-PropertyValue $cached 'LastWriteTicks') -eq $file.LastWriteTimeUtc.Ticks -and
        (Get-PropertyValue $cached 'HeadSha') -eq $HeadSha) {
        return Get-PropertyValue $cached 'Evidence'
    }

    $planRead = $false
    $headRead = $false
    $headToolIds = New-Object 'System.Collections.Generic.HashSet[string]'
    $cronListToolIds = New-Object 'System.Collections.Generic.HashSet[string]'
    $questionToolIds = New-Object 'System.Collections.Generic.HashSet[string]'
    $contracts = New-Object System.Collections.ArrayList
    $codexContracts = New-Object System.Collections.ArrayList
    $readPaths = New-Object System.Collections.ArrayList
    $controlMinutes = 0
    foreach ($line in [IO.File]::ReadLines($TranscriptPath, [Text.Encoding]::UTF8)) {
        if ($line.IndexOf('"tool_use"', [StringComparison]::Ordinal) -lt 0 -and
            $line.IndexOf('"tool_result"', [StringComparison]::Ordinal) -lt 0 -and
            $line.IndexOf('"compact', [StringComparison]::OrdinalIgnoreCase) -lt 0) { continue }
        try { $entry = $line | ConvertFrom-Json } catch { continue }
        $subtype = [string](Get-PropertyValue $entry 'subtype')
        if ($subtype -match 'compact') {
            $planRead = $false
            $headRead = $false
            $headToolIds.Clear()
            $cronListToolIds.Clear()
            $questionToolIds.Clear()
            $contracts.Clear()
            $codexContracts.Clear()
            $readPaths.Clear()
            $controlMinutes = 0
        }
        $message = Get-PropertyValue $entry 'message'
        $content = Get-PropertyValue $message 'content'
        foreach ($item in @($content)) {
            $itemType = [string](Get-PropertyValue $item 'type')
            if ($itemType -eq 'tool_use') {
                $toolName = [string](Get-PropertyValue $item 'name')
                $toolId = [string](Get-PropertyValue $item 'id')
                $inputValue = Get-PropertyValue $item 'input'
                if ($toolName -eq 'CronList') { [void]$cronListToolIds.Add($toolId) }
                if ($toolName -eq 'AskUserQuestion') { [void]$questionToolIds.Add($toolId) }
                $filePath = [string](Get-PropertyValue $inputValue 'file_path')
                if ($filePath) {
                    [void]$readPaths.Add($filePath)
                    if ($filePath -match '[\\/]docs[\\/]PLAN-STAND\.md$') { $planRead = $true }
                }
                $command = [string](Get-PropertyValue $inputValue 'command')
                if (-not $command) { continue }
                $normal = ($command -replace '[`\r\n]+', ' ' -replace '\s+', ' ').Trim()
                if ($normal -match '(Get-Content|Select-String|rg|type)\b.*docs[\\/]PLAN-STAND\.md') { $planRead = $true }
                if ($normal -match 'git(?:\.exe)?\s+(?:-C\s+\S+\s+)?rev-parse\s+HEAD') { [void]$headToolIds.Add($toolId) }
                if ($normal -match 'tools[\\/]plan[\\/]planstand\.py' -and $codexContracts.Count) {
                    $codexContracts.Clear()
                }
                if ($normal -match 'claude(?:\.exe)?\s+.*--bg(?:\s|$)' -and
                    $normal -match '--model\s+([^\s]+)' -and $normal -match '--effort\s+([^\s]+)') {
                    $modelMatch = [regex]::Match($normal, '--model\s+([^\s]+)')
                    $effortMatch = [regex]::Match($normal, '--effort\s+([^\s]+)')
                    $nameMatch = [regex]::Match($normal, '--name\s+["'']?([^\s"'']+)')
                    $workerNameMatch = if ($nameMatch.Success) {
                        [regex]::Match($nameMatch.Groups[1].Value, '^nakama-(.+)-([0-9a-fA-F]{7,40})-(bau|fort)$')
                    } else { $null }
                    if ($null -ne $workerNameMatch -and $workerNameMatch.Success -and $normal -match '--permission-mode\s+auto') {
                        [void]$contracts.Add([pscustomobject]@{
                            Name = $nameMatch.Groups[1].Value
                            Model = $modelMatch.Groups[1].Value.Trim('"', "'")
                            Effort = $effortMatch.Groups[1].Value.Trim('"', "'")
                            TicketName = $workerNameMatch.Groups[1].Value
                            BaseShaPrefix = $workerNameMatch.Groups[2].Value
                        })
                    }
                }
                if ($normal -match 'codex(?:\.ps1|\.exe)?\s+.*\bexec\b.*\b(review|resume)\b' -and
                    $normal -match '(?:^|\s)-m\s+([^\s]+)' -and $normal -match '--json(?:\s|$)') {
                    $codexModel = [regex]::Match($normal, '(?:^|\s)-m\s+([^\s]+)').Groups[1].Value.Trim('"', "'")
                    $codexEffortMatch = [regex]::Match($normal, 'model_reasoning_effort(?:=|\\?"?=\\?")([a-z]+)')
                    if (-not $codexEffortMatch.Success) {
                        $codexEffortMatch = [regex]::Match($normal, '\$solEffort\s*=\s*["'']?(high|xhigh)')
                    }
                    $codexEffort = if ($codexEffortMatch.Success) { $codexEffortMatch.Groups[1].Value } else { '?' }
                    [void]$codexContracts.Add([pscustomobject]@{
                        ToolId = $toolId
                        Model = $codexModel
                        Effort = $codexEffort
                        Mode = if ($normal -match '\breview\b') { 'Prüfer' } else { 'Nacharbeit' }
                        Completed = $false
                        ThreadSeen = $false
                        Succeeded = $false
                    })
                }
            }
            elseif ($itemType -eq 'tool_result') {
                $sourceId = [string](Get-PropertyValue $item 'tool_use_id')
                if ($headToolIds.Contains($sourceId)) {
                    $resultText = $item | ConvertTo-Json -Depth 12 -Compress
                    if ($resultText -match [regex]::Escape($HeadSha)) { $headRead = $true }
                }
                if ($cronListToolIds.Contains($sourceId)) {
                    $cronResultText = $item | ConvertTo-Json -Depth 12 -Compress
                    $minuteMatches = @([regex]::Matches($cronResultText, 'Every\s+(5|15|30)\s+minutes', [Text.RegularExpressions.RegexOptions]::IgnoreCase) | ForEach-Object { [int]$_.Groups[1].Value } | Select-Object -Unique)
                    if ($minuteMatches.Count -eq 1) { $controlMinutes = $minuteMatches[0] }
                    elseif ($minuteMatches.Count -gt 1) { $controlMinutes = -1 }
                    else { $controlMinutes = 0 }
                }
                if ($questionToolIds.Contains($sourceId)) { [void]$questionToolIds.Remove($sourceId) }
                foreach ($codexContract in @($codexContracts | Where-Object { $_.ToolId -eq $sourceId })) {
                    $codexContract.Completed = $true
                    $codexResultText = $item | ConvertTo-Json -Depth 12 -Compress
                    if ($codexResultText -match 'thread\.started|thread_id|threadId') { $codexContract.ThreadSeen = $true }
                    $isError = [bool](Get-PropertyValue $item 'is_error')
                    $exitMatch = [regex]::Match($codexResultText, '(?:exit_code|exit code)[^0-9-]*(-?\d+)', [Text.RegularExpressions.RegexOptions]::IgnoreCase)
                    $badExit = $exitMatch.Success -and [int]$exitMatch.Groups[1].Value -ne 0
                    $codexContract.Succeeded = $codexContract.ThreadSeen -and -not $isError -and -not $badExit
                }
            }
        }
    }
    $evidence = [pscustomobject]@{
        PlanRead = $planRead
        HeadRead = $headRead
        QuestionWaiting = ($questionToolIds.Count -gt 0)
        WorkerContracts = @($contracts)
        CodexContracts = @($codexContracts)
        ControlMinutes = $controlMinutes
        ReadPaths = @($readPaths)
    }
    $snapshot = [pscustomobject]@{
        Length = $file.Length
        LastWriteTicks = $file.LastWriteTimeUtc.Ticks
        HeadSha = $HeadSha
        Evidence = $evidence
    }
    Write-JsonFileAtomic $cachePath $snapshot
    return $evidence
}

function Format-Percent {
    param([object]$Value)
    if ($null -eq $Value) { return 'nicht verfügbar' }
    return ('{0:0} %' -f [double]$Value)
}

function Format-Reset {
    param([object]$Epoch)
    if ($null -eq $Epoch) { return 'nicht verfügbar' }
    try { return [DateTimeOffset]::FromUnixTimeSeconds([long]$Epoch).ToLocalTime().ToString('dd.MM. HH:mm') }
    catch { return 'nicht verfügbar' }
}

function Format-Tokens {
    param([long]$Value)
    if ($Value -ge 1000) { return ('{0:N0}k' -f [math]::Round($Value / 1000.0)).Replace(',', '.') }
    return [string]$Value
}

function New-Bar {
    param([double]$Percent, [int]$Width = 10)
    $bounded = [math]::Max(0, [math]::Min(100, $Percent))
    $filled = [int][math]::Floor($bounded * $Width / 100)
    return ('█' * $filled) + ('░' * ($Width - $filled))
}

function Get-AgentLabel {
    param([object]$Agent)
    $state = [string](Get-PropertyValue $Agent 'state')
    $status = [string](Get-PropertyValue $Agent 'status')
    $waiting = [string](Get-PropertyValue $Agent 'waitingFor')
    if ($waiting -or $status -eq 'waiting') { return 'braucht Eingabe' }
    switch ($state) {
        'working' { return 'arbeitet' }
        'blocked' { return 'blockiert' }
        'done' { return 'beendet' }
        'failed' { return 'fehlgeschlagen' }
        'stopped' { return 'gestoppt' }
        default { if ($status) { return $status }; return 'unbekannt' }
    }
}

function Show-Plan {
    $snapshot = Get-PlanSnapshot
    if (-not $snapshot.Ok) {
        Write-Output "! PLAN NICHT LESBAR · $($snapshot.Error)"
        exit 3
    }
    if ($snapshot.DirtySources -or $snapshot.SourceSha -ne $snapshot.CurrentSourceSha) {
        Write-Output "! PLANQUELLEN NICHT FRISCH · erst tools/plan/planstand.py ausführen"
        exit 4
    }
    $knownSteps = @($snapshot.Plan.phasen | ForEach-Object { @($_.schritte) } | ForEach-Object { [string]$_.id })
    if ($CurrentStep -and $knownSteps -notcontains $CurrentStep) {
        Write-Output "! UNBEKANNTES ARBEITSPAKET · $CurrentStep"
        exit 5
    }
    Write-Output "NAKAMA PLAN · $($snapshot.Accepted) / $($snapshot.Total) fertig · $($snapshot.Open) offen"
    foreach ($phase in @($snapshot.Plan.phasen)) {
        Write-Output ''
        Write-Output ([string]$phase.titel).ToUpperInvariant()
        foreach ($step in @($phase.schritte)) {
            $status = [string]$snapshot.StatusById[[string]$step.id]
            $symbol = switch ($status) {
                'abgenommen' { '✓' }
                'gebaut' { '◐' }
                default { '○' }
            }
            if ($CurrentStep -and $CurrentStep -eq [string]$step.id -and $status -ne 'abgenommen') { $symbol = '→' }
            $suffix = if ($status -eq 'gebaut') { ' — abschließende Prüfung fehlt' } else { '' }
            Write-Output "$symbol $($step.leitungsname)$suffix"
        }
    }
    Write-Output ''
    Write-Output "Fertig bedeutet: umgesetzt, aktuell belegt und auf der geforderten Stufe abgenommen."
}

function Show-StatusLine {
    try {
        $raw = [Console]::In.ReadToEnd()
        if (-not $raw.Trim()) { throw 'kein StatusLine-JSON auf stdin' }
        $inputData = $raw | ConvertFrom-Json
        $sessionId = [string](Get-PropertyValue $inputData 'session_id')
        if (-not $sessionId) { $sessionId = 'unknown' }
        $sessionName = [string](Get-PropertyValue $inputData 'session_name')
        $modelName = [string](Get-NestedValue $inputData @('model', 'display_name'))
        $modelId = [string](Get-NestedValue $inputData @('model', 'id'))
        $effort = [string](Get-NestedValue $inputData @('effort', 'level'))
        $planSnapshot = Get-PlanSnapshot

        if ($sessionName -notlike 'nakama-dirigent*') {
            $planText = if ($planSnapshot.Ok) { "$($planSnapshot.Accepted)/$($planSnapshot.Total) fertig" } else { 'Plan nicht lesbar' }
            Write-Output "NAKAMA · $modelName/$effort · $planText · Dirigent über /dirigent"
            return
        }

        $agentsSnapshot = Get-AgentSnapshot $sessionId
        $claudeRate = Get-ClaudeRateSnapshot $inputData $sessionId
        $codexRate = Get-CodexRateSnapshot $sessionId
        $headRead = Invoke-GitText 'rev-parse HEAD'
        $headSha = if ($headRead.Ok) { $headRead.StdOut.Trim() } else { '' }
        $transcriptPath = [string](Get-PropertyValue $inputData 'transcript_path')
        $evidence = Get-TranscriptEvidence $transcriptPath $sessionId $headSha
        $processRows = @()
        try {
            $processRows = @(Get-CimInstance Win32_Process -ErrorAction Stop | Where-Object { $_.CommandLine })
        }
        catch { }
        $observerCandidates = @($processRows | Where-Object {
            $_.ProcessId -ne $PID -and $_.CommandLine -match 'cockpit\.ps1.+-WatchWorker'
        })
        $codexProcess = @($processRows | Where-Object {
            $_.CommandLine -match 'codex(?:\.ps1|\.exe)?\s+.*\bexec\b.*\b(review|resume)\b.*--json'
        }) | Select-Object -First 1
        $latestCodex = @($evidence.CodexContracts) | Select-Object -Last 1

        $participantCandidates = @()
        if ($agentsSnapshot.Ok) {
            $participantCandidates = @($agentsSnapshot.Agents | Where-Object {
                $name = [string](Get-PropertyValue $_ 'name')
                $cwd = [string](Get-PropertyValue $_ 'cwd')
                $sid = [string](Get-PropertyValue $_ 'sessionId')
                $name -like 'nakama-*' -and $name -notlike 'nakama-dirigent*' -and
                    $sid -ne $sessionId -and
                    ([IO.Path]::GetFullPath($cwd).TrimEnd('\') -eq $script:RepoRoot.TrimEnd('\'))
            })
        }
        $contractNames = @($evidence.WorkerContracts | ForEach-Object { [string](Get-PropertyValue $_ 'Name') })
        $participants = @($participantCandidates | Where-Object {
            $state = [string](Get-PropertyValue $_ 'state')
            $status = [string](Get-PropertyValue $_ 'status')
            $name = [string](Get-PropertyValue $_ 'name')
            $state -in @('working', 'blocked') -or $status -eq 'waiting' -or $contractNames -contains $name
        })
        $activeParticipants = @($participants | Where-Object {
            $state = [string](Get-PropertyValue $_ 'state')
            $state -in @('working', 'blocked') -or [string](Get-PropertyValue $_ 'status') -eq 'waiting'
        })
        $activeWorkerId = if ($activeParticipants.Count -eq 1) { [string](Get-PropertyValue $activeParticipants[0] 'id') } else { '' }
        $observer = @($observerCandidates | Where-Object {
            (Test-ProcessArgument $_.CommandLine 'DirigentSessionId' $sessionId) -and
                (-not $activeWorkerId -or (Test-ProcessArgument $_.CommandLine 'WorkerId' $activeWorkerId))
        }) | Select-Object -First 1
        $questionWaiting = [bool](Get-PropertyValue $evidence 'QuestionWaiting')

        $contextSize = Get-NestedValue $inputData @('context_window', 'context_window_size')
        $contextPercent = Get-NestedValue $inputData @('context_window', 'used_percentage')
        $contextTokens = Get-NestedValue $inputData @('context_window', 'total_input_tokens')
        if ($null -eq $contextPercent) { $contextPercent = 0 }
        if ($null -eq $contextTokens -and $null -ne $contextSize) {
            $contextTokens = [math]::Round([double]$contextSize * [double]$contextPercent / 100)
        }

        $issues = New-Object System.Collections.ArrayList
        if ($modelId -notmatch 'fable' -and $modelName -notmatch 'Fable') { [void]$issues.Add('Modell ist nicht Fable') }
        if ($effort -ne 'xhigh') { [void]$issues.Add('Effort ist nicht xhigh') }
        if ([long]$contextSize -ne 1000000) { [void]$issues.Add('Kontextfenster ist nicht 1.000.000') }
        if (-not $planSnapshot.Ok -or $planSnapshot.DirtySources -or $planSnapshot.SourceSha -ne $planSnapshot.CurrentSourceSha) {
            [void]$issues.Add('Planstand ist nicht frisch')
        }
        if (-not $agentsSnapshot.Ok) { [void]$issues.Add('Agentzustand nicht verfügbar') }
        if (-not $codexRate.Available) { [void]$issues.Add('Codex-Wochenwert nicht verfügbar') }
        if (-not $claudeRate.Available) { [void]$issues.Add('Claude-Kontingente nicht verfügbar') }
        if (-not $planSnapshot.QuestionsAvailable) { [void]$issues.Add('Fragenquelle nicht verfügbar') }
        if (-not $evidence.PlanRead -or -not $evidence.HeadRead) { [void]$issues.Add('Arbeitsanker nicht bestätigt') }
        if ($activeParticipants.Count -gt 1) { [void]$issues.Add('mehr als ein aktiver Worker') }
        if ($activeParticipants.Count -eq 1 -and $null -eq $observer) { [void]$issues.Add('Ereignisbeobachter fehlt') }
        if ($activeParticipants.Count -eq 0 -and $null -ne $observer) { [void]$issues.Add('Ereignisbeobachter ohne Worker') }
        if ($activeParticipants.Count -eq 1 -and [int]$evidence.ControlMinutes -notin @(5, 15, 30)) { [void]$issues.Add('Kontrollloop unbestätigt') }
        if ($activeParticipants.Count -eq 0 -and [int]$evidence.ControlMinutes -ne 0) { [void]$issues.Add('Kontrollloop ohne Worker') }
        if ($planSnapshot.WorktreeCount -gt 1) { [void]$issues.Add('zusätzliches Worktree erkannt') }
        if ($planSnapshot.DirtyTree -and $activeParticipants.Count -eq 0) { [void]$issues.Add('Änderungen ohne sichtbaren Worker') }
        foreach ($agent in $participants) {
            $agentName = [string](Get-PropertyValue $agent 'name')
            $contract = @($evidence.WorkerContracts | Where-Object { [string](Get-PropertyValue $_ 'Name') -eq $agentName }) | Select-Object -Last 1
            if ($null -eq $contract) { [void]$issues.Add("Startbeleg fehlt: $agentName") }
        }
        if ($null -ne $latestCodex) {
            if (-not [bool](Get-PropertyValue $latestCodex 'ThreadSeen')) { [void]$issues.Add('Codex-Thread-ID fehlt') }
            if ([bool](Get-PropertyValue $latestCodex 'Completed') -and -not [bool](Get-PropertyValue $latestCodex 'Succeeded')) {
                [void]$issues.Add('Codex-Lauf fehlgeschlagen')
            }
            if (-not [bool](Get-PropertyValue $latestCodex 'Completed') -and $null -eq $codexProcess) {
                [void]$issues.Add('Codex-Prozess unerwartet verschwunden')
            }
        }
        if ([double]$contextPercent -ge 85) { [void]$issues.Add('Kontextgrenze erreicht') }
        foreach ($usage in @($claudeRate.FiveUsed, $claudeRate.SevenUsed, $codexRate.Used)) {
            if ($null -ne $usage -and [double]$usage -ge 85) { [void]$issues.Add('Kontingentgrenze erreicht'); break }
        }

        $top = if ($questionWaiting) { '? ENTSCHEIDUNG WARTET' } elseif ($issues.Count) { '! EINGRIFF NÖTIG' } else { '✓ ALLES IN SPUR' }
        $supervision = '—'
        $controlText = 'keine laufende Arbeitsphase'
        $minutes = [int]$evidence.ControlMinutes
        if ($minutes -in @(5, 15, 30)) {
            $supervision = switch ($minutes) { 30 { 'LOCKER' }; 5 { 'ENG' }; default { 'NORMAL' } }
            $now = Get-Date
            $delta = $minutes - ($now.Minute % $minutes)
            if ($delta -eq 0) { $delta = $minutes }
            $next = (Get-Date -Hour $now.Hour -Minute $now.Minute -Second 0).AddMinutes($delta)
            $remaining = [math]::Max(0, [int][math]::Ceiling(($next - $now).TotalMinutes))
            $controlText = "nächste Kontrolle $($next.ToString('HH:mm')) · in $remaining min"
        }
        elseif ($null -ne $observer) {
            if ($observer.CommandLine -match '-Aufsicht\s+(LOCKER|NORMAL|ENG)') { $supervision = $Matches[1] }
            $controlText = 'Kontrollloop nicht bestätigt'
        }

        Write-Output ('NAKAMA DIRIGENT'.PadRight(45) + $top)
        Write-Output "AUFSICHT $supervision · $controlText · wichtige Events sofort"
        Write-Output ''
        $anchor = if ($evidence.PlanRead -and $evidence.HeadRead) { 'Kontext ✓ Plan + HEAD' } else { 'Kontext ? Arbeitsanker' }
        Write-Output ("Fable       Dirigent · $modelName/$effort · aktiv  $anchor")
        foreach ($agent in $participants) {
            $agentName = [string](Get-PropertyValue $agent 'name')
            $contract = @($evidence.WorkerContracts | Where-Object { [string](Get-PropertyValue $_ 'Name') -eq $agentName }) | Select-Object -Last 1
            $start = if ($null -ne $contract) {
                "Start $([string](Get-PropertyValue $contract 'Model'))/$([string](Get-PropertyValue $contract 'Effort'))"
            } else { 'Start ?/?' }
            $agentAnchor = if ($null -ne $contract) { 'Kontext ✓ Auftrag' } else { 'Kontext ? Startbeleg' }
            Write-Output ("Claude      Bauer · $start · $(Get-AgentLabel $agent)  $agentAnchor")
        }
        if ($null -ne $latestCodex) {
            $codexModel = [string](Get-PropertyValue $latestCodex 'Model')
            if ($codexModel -match 'sol') { $codexModel = 'Sol' }
            $codexEffort = [string](Get-PropertyValue $latestCodex 'Effort')
            $codexState = if ([bool](Get-PropertyValue $latestCodex 'Completed')) {
                if ([bool](Get-PropertyValue $latestCodex 'Succeeded')) { 'beendet' } else { 'fehlgeschlagen' }
            } elseif ($null -ne $codexProcess) { 'prüft' } else { 'unterbrochen' }
            $codexAnchor = if ([bool](Get-PropertyValue $latestCodex 'ThreadSeen')) { 'Kontext ✓ Prüfbereich' } else { 'Kontext ? Thread-ID' }
            Write-Output ("Codex       $([string](Get-PropertyValue $latestCodex 'Mode')) · Start $codexModel/$codexEffort · $codexState  $codexAnchor")
        }
        Write-Output ''
        $contextDisplay = if ($null -ne $contextSize) {
            "$(Format-Tokens ([long]$contextTokens)) / $(Format-Tokens ([long]$contextSize))  [$(New-Bar ([double]$contextPercent))] $([math]::Round([double]$contextPercent)) %"
        } else { 'nicht verfügbar' }
        Write-Output "FABLE-KONTEXT  $contextDisplay"
        $claudeAge = (Get-UnixNow) - [long]$claudeRate.At
        $claudeText = if ($claudeRate.Available) {
            "5 Stunden $(Format-Percent $claudeRate.FiveUsed) · Reset $(Format-Reset $claudeRate.FiveReset) · " +
                "Woche $(Format-Percent $claudeRate.SevenUsed) · Reset $(Format-Reset $claudeRate.SevenReset) · vor $claudeAge s"
        } else { 'nicht verfügbar' }
        Write-Output "CLAUDE          $claudeText"
        $codexAge = (Get-UnixNow) - [long]$codexRate.At
        $codexText = if ($codexRate.Available) {
            "Woche $(Format-Percent $codexRate.Used) · Reset $(Format-Reset $codexRate.Reset) · vor $codexAge s"
        } else { "nicht verfügbar · vor $codexAge s" }
        Write-Output "CODEX           $codexText"
        if ($planSnapshot.Ok) {
            $planPercent = if ($planSnapshot.Total) { 100.0 * $planSnapshot.Accepted / $planSnapshot.Total } else { 0 }
            Write-Output "PLAN            $($planSnapshot.Accepted) / $($planSnapshot.Total) fertig · $($planSnapshot.Open) offen  [$(New-Bar $planPercent)]"
        } else {
            Write-Output 'PLAN            nicht verfügbar'
        }
        $questionText = if ($questionWaiting) {
            'Antwort in dieser Sitzung erwartet'
        } elseif (-not $planSnapshot.QuestionsAvailable) {
            'nicht verfügbar'
        } elseif ($planSnapshot.Questions) {
            "$($planSnapshot.QuestionText) · /fragen"
        } else { 'keine offene Frage' }
        Write-Output "FRAGE           $questionText          Plan: /dirigent stand"
        if (-not $questionWaiting -and $issues.Count) { Write-Output "HINWEIS         $($issues[0])" }

        $telemetry = [pscustomobject]@{
            At = Get-UnixNow
            ContextPercent = [double]$contextPercent
            ContextAvailable = ($null -ne $contextSize)
            ClaudeAvailable = [bool]$claudeRate.Available
            CodexAvailable = [bool]$codexRate.Available
            ClaudeFiveUsed = $claudeRate.FiveUsed
            ClaudeSevenUsed = $claudeRate.SevenUsed
            CodexSevenUsed = $codexRate.Used
            Issues = @($issues)
        }
        Write-JsonFileAtomic (Get-CachePath $sessionId 'telemetry') $telemetry
    }
    catch {
        Write-Output ('NAKAMA DIRIGENT'.PadRight(45) + '! EINGRIFF NÖTIG')
        Write-Output "STATUSQUELLE     nicht verfügbar · $($_.Exception.Message)"
    }
}

function Watch-WorkerState {
    $started = Get-Date
    $lastWorkerState = ''
    $lastHead = ''
    $lastWorktreeCount = -1
    $lastTelemetryState = ''
    $baseCheck = Invoke-GitText "merge-base --is-ancestor $BaseSha HEAD"
    if (-not $baseCheck.Ok) {
        Write-Output "REPO · Basis $BaseSha ist kein Vorfahr von HEAD"
        exit 8
    }
    while ($true) {
        if ($TimeoutSeconds -gt 0 -and ((Get-Date) - $started).TotalSeconds -ge $TimeoutSeconds) {
            Write-Output "WORKER $WorkerId · Beobachtungszeit abgelaufen"
            exit 7
        }
        $snapshot = Get-AgentSnapshot $DirigentSessionId -NoCache
        if (-not $snapshot.Ok) {
            $state = "Statusquelle unbekannt: $($snapshot.Error)"
            if ($state -ne $lastWorkerState) { Write-Output "WORKER $WorkerId · $state"; $lastWorkerState = $state }
            Start-Sleep -Seconds $PollSeconds
            continue
        }
        $agent = @($snapshot.Agents | Where-Object { [string](Get-PropertyValue $_ 'id') -eq $WorkerId }) | Select-Object -First 1
        if ($null -eq $agent) {
            Write-Output "WORKER $WorkerId · unerwartet verschwunden"
            exit 6
        }
        $agentCwd = [string](Get-PropertyValue $agent 'cwd')
        if (-not $agentCwd -or [IO.Path]::GetFullPath($agentCwd).TrimEnd('\') -ne $script:RepoRoot.TrimEnd('\')) {
            Write-Output "WORKER $WorkerId · unerwarteter Arbeitsbaum"
            exit 8
        }
        $state = Get-AgentLabel $agent
        if ($state -ne $lastWorkerState) {
            Write-Output "WORKER $WorkerId · $state · Start $StartModel/$StartEffort · Aufsicht $Aufsicht"
            $lastWorkerState = $state
        }

        $head = Invoke-GitText 'rev-parse HEAD'
        if ($head.Ok) {
            $currentHead = $head.StdOut.Trim()
            if ($lastHead -and $currentHead -ne $lastHead) { Write-Output "REPO · HEAD $lastHead → $currentHead" }
            if (-not $lastHead -or $currentHead -ne $lastHead) {
                $baseCheck = Invoke-GitText "merge-base --is-ancestor $BaseSha HEAD"
                if (-not $baseCheck.Ok) {
                    Write-Output "REPO · HEAD liegt nicht mehr auf Basis $BaseSha"
                    exit 8
                }
            }
            $lastHead = $currentHead
        }
        $worktrees = Invoke-GitText 'worktree list --porcelain'
        if ($worktrees.Ok) {
            $count = @(($worktrees.StdOut -split "`r?`n") | Where-Object { $_ -match '^worktree ' }).Count
            if ($lastWorktreeCount -ge 0 -and $count -ne $lastWorktreeCount) { Write-Output "REPO · Worktrees $lastWorktreeCount → $count" }
            if ($count -gt 1 -and $count -ne $lastWorktreeCount) { Write-Output "REPO · unerwartetes zusätzliches Worktree" }
            $lastWorktreeCount = $count
        }

        if ($DirigentSessionId) {
            $telemetryPath = Get-CachePath $DirigentSessionId 'telemetry'
            $telemetry = Read-JsonFile $telemetryPath
            $telemetryState = 'gesund'
            $at = Get-PropertyValue $telemetry 'At'
            if ($null -eq $at -or (Get-UnixNow) - [long]$at -gt 60 -or
                -not [bool](Get-PropertyValue $telemetry 'ContextAvailable') -or
                -not [bool](Get-PropertyValue $telemetry 'ClaudeAvailable') -or
                -not [bool](Get-PropertyValue $telemetry 'CodexAvailable')) {
                $telemetryState = 'Statusquelle unbekannt'
            }
            elseif ([double](Get-PropertyValue $telemetry 'ContextPercent') -ge 85 -or
                [double](Get-PropertyValue $telemetry 'ClaudeFiveUsed') -ge 85 -or
                [double](Get-PropertyValue $telemetry 'ClaudeSevenUsed') -ge 85 -or
                [double](Get-PropertyValue $telemetry 'CodexSevenUsed') -ge 85) {
                $telemetryState = 'Kontext- oder Kontingentgrenze erreicht'
            }
            if ($telemetryState -ne $lastTelemetryState) {
                Write-Output "TELEMETRIE · $telemetryState"
                $lastTelemetryState = $telemetryState
            }
        }

        $rawState = [string](Get-PropertyValue $agent 'state')
        if ($rawState -in @('done', 'failed', 'stopped')) {
            if ($rawState -eq 'done') { exit 0 }
            if ($rawState -eq 'stopped') { exit 5 }
            exit 4
        }
        Start-Sleep -Seconds $PollSeconds
    }
}

switch ($PSCmdlet.ParameterSetName) {
    'StatusLine' { Show-StatusLine; break }
    'Plan' { Show-Plan; break }
    'WatchWorker' { Watch-WorkerState; break }
    default { throw 'Genau einen Modus wählen: -StatusLine, -WatchWorker oder -Plan.' }
}
