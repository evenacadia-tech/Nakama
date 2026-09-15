#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1: zaehlt je Bein einer Kanon-Rohausgabe die Pruefzeilen
    mit ok-Marke (Riegel R4, Beinzahlen vorher gegen nachher).

.DESCRIPTION
    Dieselbe Regel fuer beide Laeufe: innerhalb eines Abschnitts
    "#### <Kuerzel> | <Name>" zaehlt eine Zeile, wenn sie nach Leerraum mit
    "ok" (Wortgrenze), "[ok]" oder "[OK]" beginnt oder als cargo-Testzeile mit
    "... ok" endet. Der Abschnitt "### Bau vor dem Lauf" beendet die Zaehlung.
    Die Zahl ist ein Vergleichsmass zwischen zwei Rohausgaben desselben
    Runners, keine Pruefungszahl eines einzelnen Beins.

.PARAMETER Rohausgabe
    Pfad zur Rohausgabe (docs/beweise/roh/<TICKET>-<sha7>[-dirty].md).
#>
[CmdletBinding()]
param([Parameter(Mandatory)][string] $Rohausgabe)

Set-StrictMode -Version Latest
$zaehler = [ordered]@{}
$aktuell = $null
foreach ($z in (Get-Content -LiteralPath $Rohausgabe -Encoding utf8)) {
    if ($z -match '^#### (\S+) \| (.+)$') {
        $aktuell = '{0} | {1}' -f $Matches[1], $Matches[2].Trim()
        $zaehler[$aktuell] = 0
        continue
    }
    if ($z -match '^### ') {
        if ($z -match 'Bau vor dem Lauf') { $aktuell = $null }
        continue
    }
    if ($null -eq $aktuell) { continue }
    if ($z -match '^\s*(\[ok\]|\[OK\]|ok\b)' -or $z -match '\.\.\. ok\s*$') { $zaehler[$aktuell]++ }
}
$summe = 0
foreach ($k in $zaehler.Keys) {
    '{0} | {1}' -f $k, $zaehler[$k]
    $summe += $zaehler[$k]
}
'Beine: {0} | ok-Zeilen gesamt: {1}' -f $zaehler.Count, $summe
