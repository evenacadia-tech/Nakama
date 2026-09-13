# NAK-283 Etappe 5 — Riegel M-55 (b): kVerteilungPlaetze bleibt 64 (Bauer, 13.09.2026)
#
# Matrix M-55, Testspalte woertlich: "Speicherseite als Riegel:
# `git grep -n "kVerteilungPlaetze = " eq-copilot/plugin/core/` zeigt
# unveraendert 64". Die Konstante ist die Ressourcengrenze der
# BANDverteilungen (221 Baender x 64 float, SONDE-013 M-05); die
# Headroomverteilung einer Passage waechst seit NAK-283 F08 ueber ein eigenes
# Histogramm und braucht sie nicht zu vergroessern.
#
# R1  genau EINE Definition unter eq-copilot/plugin/core/
# R2  sie lautet `kVerteilungPlaetze = 64;`
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-riegel-M-55.ps1              (Arbeitsbaum)
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-riegel-M-55.ps1 -Revision <sha>
# Exit 0 = alle Pruefungen ok, 1 = eine Pruefung rot.

[CmdletBinding()]
param(
    [string] $Wurzel   = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Revision = ''
)

$ErrorActionPreference = 'Stop'
$argumente = @('-C', $Wurzel, 'grep', '-n', 'kVerteilungPlaetze = ')
if ($Revision) { $argumente += $Revision }
$argumente += @('--', 'eq-copilot/plugin/core/')
$treffer = @(& git @argumente 2>&1 | ForEach-Object { "$_" } | Where-Object { $_ -ne '' })
$quelle = if ($Revision) { "Revision $Revision" } else { 'Arbeitsbaum' }
Write-Host "Riegel M-55 (b) gegen $quelle - git $($argumente -join ' ')"
foreach ($t in $treffer) { Write-Host "  Treffer: $t" }

$rot = 0
if ($treffer.Count -eq 1) { Write-Host "[ok]   R1 genau eine Definition von kVerteilungPlaetze unter eq-copilot/plugin/core/" }
else { Write-Host "[ROT]  R1 genau eine Definition von kVerteilungPlaetze unter eq-copilot/plugin/core/ - gefunden: $($treffer.Count)"; $rot++ }

$vierundsechzig = @($treffer | Where-Object { $_ -match 'kVerteilungPlaetze = 64;' })
if ($treffer.Count -ge 1 -and $vierundsechzig.Count -eq $treffer.Count) { Write-Host "[ok]   R2 die Definition lautet kVerteilungPlaetze = 64 - der Bandspeicher je Instanz waechst nicht" }
else { Write-Host "[ROT]  R2 die Definition lautet kVerteilungPlaetze = 64 - der Bandspeicher je Instanz waechst nicht"; $rot++ }

Write-Host ("URTEIL: {0} von 2 Pruefungen ok" -f (2 - $rot))
if ($rot -gt 0) { exit 1 }
exit 0
