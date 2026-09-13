# NAK-283 Etappe 4 — Riegel M-43 (Uebergang F04 + F05 + F09 + F12), Bauer, 13.09.2026
#
# Zusage (docs/beweise/NAK-283.md §5.3, M-43): keine Flaeche der Sonde
# begruendet sich mehr mit einem Passthrough, den es nicht mehr gibt. Die vier
# Selbstzusagen (Buslayout, Lebenszyklus, Analysezufuehrung, Zahlenkante)
# nennen den rechnenden Kern; `git grep "waere kein Passthrough"` unter
# eq-copilot/plugin/sonde/ trifft nur die nachgezogene Fassung; die
# Runner-Behauptung von A16 nennt den aktiven Kern. Riegel, kein Laufzeittest.
#
# Jede Pruefung liest den Quelltext an genau der Stelle, die ihre Zusage traegt,
# und faellt, wenn die alte Begruendung dort noch steht. `-Revision <sha>`
# misst einen committeten Stand statt des Arbeitsbaums (Rotbeweis am Basis-SHA).
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-4-riegel-M-43.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-4-riegel-M-43.ps1 -Revision 0f0bc1ed
# Exit 0 nur bei "RIEGEL M-43 OK".

[CmdletBinding()]
param(
    [string] $Revision = '',
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
)

$ErrorActionPreference = 'Stop'
$fehler = 0
$geprueft = 0

function Pruefe([bool] $ok, [string] $was, [string] $detail = '') {
    $script:geprueft++
    if (-not $ok) { $script:fehler++ }
    $zeile = ($(if ($ok) { '  ok      ' } else { '  FEHLER  ' })) + $was
    if ($detail) { $zeile += "  [$detail]" }
    Write-Output $zeile
}

function Quelltext([string] $pfad) {
    if ($Revision) {
        $roh = & git -C $Wurzel show "$($Revision):$pfad" 2>$null
        if ($LASTEXITCODE -ne 0) { throw "git show $($Revision):$pfad fehlgeschlagen" }
        return ($roh -join "`n")
    }
    return [System.IO.File]::ReadAllText((Join-Path $Wurzel $pfad))
}

# Alte Begruendungen stehen im Quelltext ueber Zeilenumbrueche verteilt. Fuer
# die Suche nach ihrem Wortlaut werden Kommentarzeichen und Leerraum deshalb auf
# ein Leerzeichen gefaltet; Code-Pruefungen lesen den Rohtext.
function Gefaltet([string] $text) {
    $t = [regex]::Replace($text, '\s*//\s*', ' ')
    return [regex]::Replace($t, '\s+', ' ')
}

$stand = if ($Revision) { "Revision $Revision" } else { 'Arbeitsbaum' }
Write-Output "NAK-283 Etappe 4 - Riegel M-43 ($stand)"

$sondeCpp = Quelltext 'eq-copilot/plugin/sonde/SondeProcessor.cpp'
$sondeH   = Quelltext 'eq-copilot/plugin/sonde/SondeProcessor.h'
$markH    = Quelltext 'eq-copilot/plugin/src/HoerMarkierung.h'
$runner   = Quelltext 'tools/beweise.ps1'
$sondeCppGefaltet = Gefaltet $sondeCpp
$sondeHGefaltet   = Gefaltet $sondeH

# R1 - F04, Buslayout (SondeProcessor.cpp, isBusesLayoutSupported)
$grepArgs = @('-C', $Wurzel, 'grep', '-n', 'waere kein Passthrough')
if ($Revision) { $grepArgs += $Revision }
$grepArgs += @('--', 'eq-copilot/plugin/sonde/')
$treffer = @(& git @grepArgs 2>$null)
$alt = @($treffer | Where-Object { $_ -notmatch 'traegt seit SONDE-015 Etappe 4a nicht mehr' })
Pruefe ($treffer.Count -ge 1 -and $alt.Count -eq 0) `
    'R1 F04: git grep "waere kein Passthrough" unter sonde/ trifft nur die nachgezogene Fassung (Marker "traegt seit SONDE-015 Etappe 4a nicht mehr")' `
    ("{0} Treffer, davon {1} ohne Marker{2}" -f $treffer.Count, $alt.Count, $(if ($alt.Count) { ': ' + ($alt -join ' | ') } else { '' }))
# Am Basis-SHA steht die alte Begruendung ueber einen Zeilenumbruch ("waere" /
# "kein Passthrough") - git grep allein faende sie dort nicht. Deshalb
# zusaetzlich ihr ganzer Wortlaut im gefalteten Text.
$altF04 = 'Ein Passthrough, der Kanaele erfinden oder verschlucken muesste, waere kein Passthrough'
Pruefe (-not $sondeCppGefaltet.Contains($altF04)) `
    'R1 F04: isBusesLayoutSupported traegt die alte Passthrough-Begruendung nicht mehr' `
    $(if ($sondeCppGefaltet.Contains($altF04)) { "alte Fassung steht: '$altF04'" } else { '' })
$regelWieGen = $sondeCpp.Contains('return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();')
Pruefe $regelWieGen 'R1 F04: die Layoutregel nennt Mono oder Stereo wie Gen (Hostbruecke.cpp)'

# R2 - F05, Lebenszyklus (SondeProcessor.h/.cpp)
$altF05 = 'Gegenstueck zu prepareToPlay (Beziehungen mitpruefen): der Kern gibt seine Blockpuffer frei'
Pruefe ($sondeH.Contains('void reset() override;') -and $sondeCpp.Contains('void SondeProcessor::reset()')) `
    'R2 F05: der dritte Hosteintritt reset ist ueberschrieben'
Pruefe (-not $sondeCppGefaltet.Contains($altF05)) `
    'R2 F05: releaseResources begruendet sich nicht mehr als Paar ohne reset' `
    $(if ($sondeCppGefaltet.Contains($altF05)) { "alte Fassung steht: '$altF05'" } else { '' })

# R3 - F09, Analysezufuehrung (SondeProcessor.cpp, processBlock)
$altF09 = 'Rechnet der Kern, ist der Tap dieses Blocks verworfen und gezaehlt (M-48) - die Analyse faellt, nie Audio.'
Pruefe ($sondeCpp.Contains('analyseQueue.verwirfOhneAudio (samples);')) `
    'R3 F09: der dritte Zustand (Tap ungueltig, Kern rechnet) fuehrt die Zeitbuchfuehrung der Queue'
Pruefe (-not $sondeCppGefaltet.Contains($altF09)) `
    'R3 F09: der Passthrough-Zweig traegt nicht mehr die Begruendung, die Analyse falle still' `
    $(if ($sondeCppGefaltet.Contains($altF09)) { 'alte Fassung steht' } else { '' })

# R4 - F12, Zahlenkante (HoerMarkierung.h, tdf2Lauf)
Pruefe ($markH.Contains('const float f = (float) y;') -and -not $markH.Contains('if (std::isfinite (y))')) `
    'R4 F12: tdf2Lauf prueft den verengten Wert, nicht seine double-Vorstufe'
Pruefe ($markH.Contains('NAK-283 F12')) 'R4 F12: die Selbstzusage des Riegels nennt die Verengung (NAK-283 F12)'

# R5 - systematisch: Kopf von SondeProcessor.h
foreach ($altKopf in @('es tut deshalb dasselbe wie Suna: nichts am Audio', 'beide Bundles sind heute Passthrough')) {
    Pruefe (-not $sondeHGefaltet.Contains($altKopf)) "R5 SondeProcessor.h traegt nicht mehr: '$altKopf'"
}

# R6 - Runner-Behauptung von A16
$a16 = ($runner -split "`n" | Where-Object { $_ -match "Kuerzel='A16'" }) -join ''
Pruefe ($a16.Contains('mit dem aktiven DSP-Kern')) 'R6 die Runner-Behauptung von A16 nennt den aktiven Kern'

Write-Output ''
if ($fehler -eq 0) {
    Write-Output "RIEGEL M-43 OK - $geprueft Pruefungen"
    exit 0
}
Write-Output "RIEGEL M-43 FEHLGESCHLAGEN - $geprueft Pruefungen, $fehler Fehler"
exit 1
