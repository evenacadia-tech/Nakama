#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1: Riegel R3 (Fixture- und Schemabaum), R4 (Zielmenge),
    R5 (nicht angefasste Pfade), R7 (.clang-tidy), R8 (NOLINT) und R9
    (identity/) als git-Messung zwischen Basis-SHA und Stand.

.DESCRIPTION
    Nur Lesen, nur Baumvergleiche (kein Index, keine Sperre). Je Riegel der
    Befehl, seine Ausgabe ("(leer)" bei leerer Ausgabe) und der Exitcode.
    git grep endet ohne Treffer mit Exit 1.
#>
[CmdletBinding()]
param(
    [string] $Basis = '432f209caf787d818790b6483bc769d54dd3912d',
    [string] $Stand = 'HEAD'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location 'C:\Users\phili\Projekte\Nakama'

function Messe([string] $titel, [string[]] $gitArgs) {
    $aus = @(& git @gitArgs 2>&1 | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    "== $titel"
    '$ git ' + ($gitArgs -join ' ')
    if ($aus.Count -eq 0 -or ($aus -join '').Trim() -eq '') { '(leer)' } else { $aus }
    "(Exit $code)"
    ''
}

$sha = (& git rev-parse $Stand).Trim()
"NAK-289 Etappe 1 - Riegelmessung $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), Basis $Basis, Stand $sha"
''
$bereich = "$Basis..$sha"
Messe 'R7 eq-copilot/plugin/.clang-tidy (Inhalt)' @('diff', $bereich, '--', 'eq-copilot/plugin/.clang-tidy')
Messe 'R5 tools/beweise.ps1' @('diff', '--stat', $bereich, '--', 'tools/beweise.ps1')
Messe 'R5 und R9 eq-copilot/identity' @('diff', '--stat', $bereich, '--', 'eq-copilot/identity')
Messe 'R3 und R5 eq-copilot/schemas' @('diff', '--stat', $bereich, '--', 'eq-copilot/schemas')
Messe 'R3 und R5 eq-copilot/fixtures' @('diff', '--stat', $bereich, '--', 'eq-copilot/fixtures')
Messe 'R5 eq-copilot/install' @('diff', '--stat', $bereich, '--', 'eq-copilot/install')
Messe 'R4 und R5 eq-copilot/plugin/CMakeLists.txt' @('diff', '--stat', $bereich, '--', 'eq-copilot/plugin/CMakeLists.txt')
Messe 'R5 broker' @('diff', '--stat', $bereich, '--', 'broker')
Messe 'R5 tools ausser tools/plan/tidy.py' @('diff', '--stat', $bereich, '--', 'tools', ':(exclude)tools/plan/tidy.py')
Messe 'R5 docs ausser Manifest und Rohdaten' @('diff', '--stat', $bereich, '--', 'docs', ':(exclude)docs/beweise/NAK-289.md', ':(exclude)docs/beweise/roh/NAK-289-*')
Messe 'R5 .claude' @('diff', '--stat', $bereich, '--', '.claude')
Messe 'R5 design' @('diff', '--stat', $bereich, '--', 'design')
Messe 'R5 fremde Ordner briefing-hub und nimbalyst-local (getrackte Dateien)' @('ls-files', '--', 'briefing-hub', 'nimbalyst-local')
Messe 'R8 NOLINT im Plugin am Stand' @('grep', '-n', '-I', 'NOLINT', $sha, '--', 'eq-copilot/plugin')
Messe 'R8 NOLINT im Plugin am Basis-SHA' @('grep', '-n', '-I', 'NOLINT', $Basis, '--', 'eq-copilot/plugin')
Messe 'Ticketpfade wie gebaut (alle Aenderungen Basis bis Stand)' @('diff', '--stat=200', $bereich)
