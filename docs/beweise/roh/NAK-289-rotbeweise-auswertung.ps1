#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1: fasst die Rohausgaben der Rotbeweise zusammen
    (docs/beweise/roh/NAK-289-rot-<klasse>.txt), je Mutation eine Zeile.

.DESCRIPTION
    Liest je Abschnitt "=============== Rotbeweis <Id>": Datei, Treffer des
    alten Textes, SHA-256 vorher, mutiert und nach Ruecknahme, Bau-Exit mit
    Mutation, Exit des Beinhelfers mit Mutation und die fallenden Pruefungen
    (Zeilen "  FEHLER  ..." oder "[ROT]  ..."). Aus der Exitcode-Datei
    docs/beweise/roh/NAK-289-etappe1-beine.txt kommen Exit und Kurzbefund des
    Pruefprogramms je Rotbeweis und die Beinlaeufe nach der Ruecknahme; aus den
    Rohausgaben die Schlussbauten. Nur Auswertung, misst nichts.

.PARAMETER Markdown
    Gibt am Ende zusaetzlich die SHA-256-Tabelle als Markdown-Zeilen aus.
#>
[CmdletBinding()]
param(
    [string] $Ordner = 'docs/beweise/roh',
    [switch] $Markdown
)

Set-StrictMode -Version Latest

$beinZeilen = @(Get-Content -LiteralPath (Join-Path $Ordner 'NAK-289-etappe1-beine.txt') -Encoding utf8 | Where-Object { $_ -match '\| Rotbeweis' })
$ergebnisse = [System.Collections.Generic.List[object]]::new()
$schluss = [System.Collections.Generic.List[string]]::new()
foreach ($datei in (Get-ChildItem -Path $Ordner -Filter 'NAK-289-rot-*.txt' | Sort-Object Name)) {
    $aktuell = $null
    foreach ($z in (Get-Content -LiteralPath $datei.FullName -Encoding utf8)) {
        if ($z -match '^=+ Rotbeweis (\S+) ') {
            $aktuell = [pscustomobject]@{ Id = $Matches[1]; Datei = ''; Treffer = ''; ShaVor = ''; ShaMut = ''; ShaNach = ''; Gleich = ''
                Bau = ''; Helfer = ''; Faellt = [System.Collections.Generic.List[string]]::new(); Befund = '' }
            $ergebnisse.Add($aktuell)
            continue
        }
        if ($z -match '^(Schlussbau .*: Exit \S+|Bein \S+ nach Ruecknahme: Exit .*)$') { $schluss.Add("$($datei.Name): $($Matches[1])"); continue }
        if ($null -eq $aktuell) { continue }
        if ($z -match '^Datei  : (\S+)') { $aktuell.Datei = $Matches[1]; continue }
        if ($z -match '^Treffer des alten Textes: (\d+)') { $aktuell.Treffer = $Matches[1]; continue }
        if ($z -match '^SHA-256 vorher : (\S+)') { $aktuell.ShaVor = $Matches[1]; continue }
        if ($z -match '^SHA-256 mutiert: (\S+)') { $aktuell.ShaMut = $Matches[1]; continue }
        if ($z -match '^Bau \S+ mit Mutation: Exit (\S+)') { $aktuell.Bau = $Matches[1]; continue }
        if ($z -match '^Bein \S+ mit Mutation: Exit (\S+)') { $aktuell.Helfer = $Matches[1]; continue }
        if ($z -match '^(BEFUND|ABBRUCH):') { $aktuell.Befund = $z; continue }
        if ($z -match '^\s*FEHLER\s{2}(.+?)(\s{2}\[|$)') { $aktuell.Faellt.Add($Matches[1].Trim()); continue }
        if ($z -match '^\[ROT\]\s+(.+?)(\s{2}\[|$)') { $aktuell.Faellt.Add($Matches[1].Trim()); continue }
        if ($z -match '^SHA-256 nach Ruecknahme: (\S+) \((.+)\)') {
            $aktuell.ShaNach = $Matches[1]; $aktuell.Gleich = $Matches[2]
            $aktuell = $null   # Abschnittsende: folgende Beinausgaben gehoeren zu keiner Mutation
            continue
        }
    }
}

foreach ($e in $ergebnisse) {
    $bein = @($beinZeilen | Where-Object { $_ -match "\| Rotbeweis $([regex]::Escape($e.Id)) mit Mutation \|" }) | Select-Object -Last 1
    $pruefprogramm = if ($bein) { $t = $bein -split ' \| ', 7; '{0}, {1} ({2})' -f $t[3], $t[4], $t[6] } else { 'keine Zeile in der Exitcode-Datei' }
    '{0} | {1} | Treffer {2} | Bau {3} | Pruefprogramm {4} | Helfer-Exit {5} | faellt ({6}): {7} | SHA-256 vorher {8} | mutiert {9} | nach Ruecknahme {10} ({11}){12}' -f
        $e.Id, $e.Datei, $e.Treffer, $e.Bau, $pruefprogramm, $e.Helfer, $e.Faellt.Count, ($e.Faellt -join ' ; '),
        $e.ShaVor, $e.ShaMut, $e.ShaNach, $e.Gleich, $(if ($e.Befund) { " | $($e.Befund)" } else { '' })
}
''
$ok = @($ergebnisse | Where-Object { $_.Treffer -eq '1' -and $_.Bau -eq '0' -and $_.Helfer -notin @('', '0') -and $_.Faellt.Count -gt 0 -and $_.Gleich -eq 'bytegleich' -and $_.ShaNach -eq $_.ShaVor -and -not $_.Befund }).Count
"Rotbeweise: $($ergebnisse.Count) | davon genau ein Treffer, Bau 0, Bein rot mit fallender Pruefung, SHA-256 nach Ruecknahme = vorher: $ok"
''
$schluss
@($beinZeilen | Where-Object { $_ -match 'nach Ruecknahme' })

if ($Markdown) {
    ''
    '| Id | Datei | SHA-256 vorher | SHA-256 mutiert | SHA-256 nach Rücknahme |'
    '|---|---|---|---|---|'
    foreach ($e in $ergebnisse) {
        '| {0} | `{1}` | `{2}` | `{3}` | `{4}` ({5}) |' -f $e.Id, ($e.Datei -replace '^eq-copilot/plugin/', ''), $e.ShaVor, $e.ShaMut, $e.ShaNach, $e.Gleich
    }
}
