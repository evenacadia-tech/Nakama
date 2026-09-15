#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1: vergleicht zwei Ausgaben von roh/NAK-289-beinzahlen.ps1
    je Bein (Riegel R4: Beinmenge und ok-Zeilen vorher gegen nachher).

.DESCRIPTION
    Nennt Beine, die nur in einer der beiden Listen stehen, jede geaenderte
    ok-Zeilenzahl mit Differenz, die Zahl der unveraenderten Beine und die
    Summen. Nur Auswertung, misst nichts.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string] $Vorher,
    [Parameter(Mandatory)][string] $Nachher
)

Set-StrictMode -Version Latest

function Lies([string] $pfad) {
    $m = [ordered]@{}
    foreach ($z in (Get-Content -LiteralPath $pfad -Encoding utf8)) {
        if ($z -match '^(\S+) \| (.+) \| (\d+)$') { $m["$($Matches[1]) | $($Matches[2])"] = [int] $Matches[3] }
    }
    $m
}

$v = Lies $Vorher
$n = Lies $Nachher
"Beine vorher: $($v.Count) | nachher: $($n.Count)"
foreach ($k in $v.Keys) { if (-not $n.Contains($k)) { "nur vorher: $k" } }
foreach ($k in $n.Keys) { if (-not $v.Contains($k)) { "nur nachher: $k" } }
$gleich = 0
foreach ($k in $n.Keys) {
    if (-not $v.Contains($k)) { continue }
    if ($v[$k] -eq $n[$k]) { $gleich++ }
    else { '{0}: {1} -> {2} (Differenz {3})' -f $k, $v[$k], $n[$k], ($n[$k] - $v[$k]) }
}
"unveraendert: $gleich"
'ok-Zeilen gesamt: {0} -> {1}' -f ($v.Values | Measure-Object -Sum).Sum, ($n.Values | Measure-Object -Sum).Sum
