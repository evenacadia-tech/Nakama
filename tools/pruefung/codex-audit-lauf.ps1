#Requires -Version 7
# Abgekoppelter Codex-Auditlauf je Phase (Tiefenaudit 3, User-Entscheid 15.09.2026).
# Baut den Auftrag aus KOPF.txt + phase-<NN>.txt, ersetzt {{HEAD}}, {{DATUM}}, {{PHASE}},
# schreibt den vollstaendigen Auftrag als Beleg nach roh/, ruft `codex exec` (kein `review`:
# der Prompt ist frei, Codex schreibt seinen Bericht selbst in den Auditordner) und legt
# JSONL, Schlussnachricht und Startlog unter $env:TEMP ab. Sandbox workspace-write, damit
# Codex genau zwei Dateien schreiben darf (Bericht, Anhang an BEFUNDE.md); der Dirigent
# prueft danach `git status --short` gegen diese Erwartung. Kein Bau, kein Test, kein Kanon.
param(
    [Parameter(Mandatory)][ValidatePattern('^\d{2}$')][string]$Phase,
    [Parameter(Mandatory)][ValidatePattern('^[A-Fa-f0-9]{7,40}$')][string]$HeadSha,
    [string]$Repo = 'C:\Users\phili\Projekte\Nakama',
    [string]$Audit = 'docs/audits/2026-09-15-tiefenaudit',
    [string]$Model = 'gpt-6-astra',   # immer Astra, Effort max oder xhigh (User-Wort 10.09.2026)
    [string]$Effort = 'max',           # nie 'ultra'
    [string]$Sandbox = 'workspace-write'
)
Set-Location $Repo
$kennung = "tiefenaudit-p$Phase"
$jsonl = Join-Path $env:TEMP "nakama-$kennung-$HeadSha.jsonl"
$last  = Join-Path $env:TEMP "nakama-$kennung-$HeadSha-last.txt"
$log   = Join-Path $env:TEMP "nakama-$kennung-$HeadSha-start.log"
$kopf  = Get-Content (Join-Path $Audit 'auftraege/KOPF.txt') -Raw -Encoding utf8
$teil  = Get-Content (Join-Path $Audit "auftraege/phase-$Phase.txt") -Raw -Encoding utf8
$head  = git rev-parse HEAD
if (-not $head.StartsWith($HeadSha)) { "HEAD $head passt nicht zu $HeadSha" | Set-Content $log; exit 3 }
$datum = Get-Date -Format 'yyyy-MM-dd HH:mm'
$text  = ($kopf + "`n" + $teil) -replace '\{\{HEAD\}\}', $head -replace '\{\{DATUM\}\}', $datum -replace '\{\{PHASE\}\}', $Phase
$beleg = Join-Path $Audit "roh/phase-$Phase-auftrag.txt"
[IO.File]::WriteAllText((Join-Path $Repo $beleg), $text, [Text.UTF8Encoding]::new($false))
"START $(Get-Date -Format o) HEAD=$head PID=$PID MODEL=$Model EFFORT=$Effort SANDBOX=$Sandbox PHASE=$Phase" | Set-Content $log
$text | codex -a never exec --ignore-user-config -m $Model -c "model_reasoning_effort=`"$Effort`"" -c 'windows.sandbox="elevated"' -C . -s $Sandbox --json -o $last - *> $jsonl
$code = $LASTEXITCODE
if (Test-Path $last) { Copy-Item $last (Join-Path $Repo "$Audit/roh/phase-$Phase-last.txt") -Force }
"ENDE $(Get-Date -Format o) EXIT=$code HEAD_NACHHER=$(git rev-parse HEAD)" | Add-Content $log
"EXIT=$code" | Add-Content $log
