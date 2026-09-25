# Codex-Bauer (Dirigenten-Skill §3.6, NAK-396): vor dem Claude-Wochenreset baut Codex gpt-5.6-sol xhigh in einer
# workspace-write-Sandbox; er stagt, committet und pusht nie (der Dirigent committet den Stand nach eigenem Kanon).
# Gemessen 25.09.2026 (NAK-401): cmake liegt nicht im Sandbox-PATH (absoluter Visual-Studio-Pfad im Auftrag nennen),
# der Store-Python 3.13 startet dort nicht (A5 mit C:\Python314\python.exe; A8/A9/A10 brauchen rfc8785 und faehrt der
# Dirigent), cargo und MSBuild laufen; shell_environment_policy.inherit=all vererbt die Umgebung.
# Aufruf: pwsh -NoProfile -File tools/dirigent/codex-bau.ps1 -Kennung <k> -Prompt <auftragsdatei> -HeadSha <sha> [-Effort xhigh]
# Log: $env:TEMP\nakama-<Kennung>-<HeadSha>-bau-start.log (endet mit EXIT=), Bericht -bau-last.txt, Rohstrom -bau.jsonl.
param(
    [Parameter(Mandatory)][string]$Kennung,
    [Parameter(Mandatory)][string]$Prompt,      # Auftragsdatei
    [Parameter(Mandatory)][string]$HeadSha,
    [string]$Repo = 'C:\Users\phili\Projekte\Nakama',
    [string]$Model = 'gpt-5.6-sol',             # Bauer vor dem Reset: Sol xhigh (User 21.09./25.09.2026)
    [string]$Effort = 'xhigh',
    [string]$Sandbox = 'workspace-write'
)
$ErrorActionPreference = 'Continue'
Set-Location $Repo
$jsonl = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha-bau.jsonl"
$last  = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha-bau-last.txt"
$log   = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha-bau-start.log"
$text  = Get-Content -Raw -Encoding UTF8 $Prompt
Add-Content $log ("START " + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + " model=$Model effort=$Effort sandbox=$Sandbox head=$HeadSha")
$text | codex -a never exec --ignore-user-config -m $Model -c "model_reasoning_effort=`"$Effort`"" -c 'windows.sandbox="elevated"' -c 'shell_environment_policy.inherit="all"' -C . -s $Sandbox --json -o $last - *> $jsonl
$code = $LASTEXITCODE
Add-Content $log ("ENDE " + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))
Add-Content $log ("EXIT=" + $code)
