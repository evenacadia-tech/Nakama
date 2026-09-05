#Requires -Version 7
# Abgekoppelter Codex-Prüflauf (Dirigent). Schreibt JSONL + Schlussurteil + Startlog in $env:TEMP.
param(
    [Parameter(Mandatory)][string]$Kennung,
    [Parameter(Mandatory)][string]$Prompt,
    [Parameter(Mandatory)][string]$HeadSha,
    [string]$Repo = 'C:\Users\phili\Projekte\Nakama',
    [string]$Model = 'gpt-6-astra',   # Astra prueft, Sol (gpt-5.6-sol) nur Gegenpruefer - User-Wort 05.09.2026
    [string]$Effort = 'max',           # nie 'ultra'
    [string]$Sandbox = 'read-only'
)
Set-Location $Repo
$jsonl = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha.jsonl"
$last  = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha-last.txt"
$log   = Join-Path $env:TEMP "nakama-$Kennung-$HeadSha-start.log"
"START $(Get-Date -Format o) HEAD=$(git rev-parse HEAD) PID=$PID MODEL=$Model EFFORT=$Effort" | Set-Content $log
$text = Get-Content $Prompt -Raw
$text | codex -a never exec --ignore-user-config -m $Model -c "model_reasoning_effort=`"$Effort`"" -c 'windows.sandbox="elevated"' -C . -s $Sandbox review --json -o $last - *> $jsonl
$code = $LASTEXITCODE
"ENDE $(Get-Date -Format o) EXIT=$code HEAD_NACHHER=$(git rev-parse HEAD)" | Add-Content $log
"EXIT=$code" | Add-Content $log
