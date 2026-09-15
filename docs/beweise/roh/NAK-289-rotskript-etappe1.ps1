#Requires -Version 7.0
<#
.SYNOPSIS
    NAK-289 Etappe 1 - Rotbeweise je Zusage.

.DESCRIPTION
    Je Mutation: Originalbytes lesen und SHA-256 festhalten; genau EINE Stelle
    per exaktem Textersatz aendern (der alte Text muss genau einmal vorkommen,
    sonst Abbruch); LastWriteTime setzen (NAK-230); das Ziel bauen; das Bein
    fahren - erwartet ROT; die Originalbytes zurueckschreiben, SHA-256 gegen
    vorher pruefen, LastWriteTime setzen. Nach allen Mutationen werden die
    beruehrten Ziele neu gebaut und die Beine gruen gefahren.

    Rohausgaben: docs/beweise/roh/NAK-289-rot-<klasse>.txt (Kopf je Mutation
    plus vollstaendige Beinausgabe); Exitcodezeilen zusaetzlich in
    docs/beweise/roh/NAK-289-etappe1-beine.txt.

    Gefahren vom Erbauer abgekoppelt; aendert nur die genannten Quellen und
    stellt sie bytegleich wieder her.

.PARAMETER Nur
    Optional: nur diese Mutations-Ids.

.NOTES
    Exit 0 = jede Mutation baut, jedes Bein ist mit Mutation rot und nach der
    Ruecknahme gruen, jede Ruecknahme bytegleich. Exit 2 sonst.
#>
[CmdletBinding()]
param([string[]] $Nur = @())

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$Wurzel = 'C:\Users\phili\Projekte\Nakama'
Set-Location $Wurzel
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$beinSkript = Join-Path $Wurzel 'docs\beweise\roh\NAK-289-beine-etappe1.ps1'
$exitDatei = 'docs/beweise/roh/NAK-289-etappe1-beine.txt'

# Alt und Neu mit LF; fuer CRLF-Dateien wird das Zeilenende angepasst.
$mutationen = @(
    [pscustomobject]@{ Id = 'a1'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/analysis/TruePeak.h'
        Zusage = 'TruePeakDetektor::vorbereiten legt beide Verlaeufe an (Erfolgspfad innerhalb der neuen Grenze)'
        Alt = "            verlaufR.assign ((std::size_t) kRing, 0.0);`n"; Neu = ''
        Ziel = 'EqCopSonde013TruePeakGoldenTest'; Bein = 'B17' }
    [pscustomobject]@{ Id = 'a2'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/analysis/TruePeak.h'
        Zusage = 'Allokationsfehler in vorbereiten endet in std::terminate, nie verschluckt'
        Alt = "        catch (...)`n        {`n            std::terminate();`n        }"; Neu = "        catch (...)`n        {`n        }"
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'a3'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/ipc/IpcQueues.h'
        Zusage = 'P1Warteschlange::bestaetigen laesst den Wiederholpuffer abfliessen (A-P1-06, Erfolgspfad innerhalb der neuen Grenze)'
        Alt = "        try`n        {`n            abfliessen();`n        }"; Neu = "        try`n        {`n        }"
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'a4'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/ipc/IpcQueues.h'
        Zusage = 'Allokationsfehler in bestaetigen endet in std::terminate, nie verschluckt'
        Alt = "        catch (...)`n        {`n            std::terminate();`n        }"; Neu = "        catch (...)`n        {`n        }"
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'a5'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/ipc/PipeToken.h'
        Zusage = 'istProbePipename: der Praefix allein ist kein Probe-Pipename (Laengenbedingung bleibt)'
        Alt = 'return name.size() > praefix.size() && std::string_view (name).starts_with (praefix);'
        Neu = 'return name.size() >= praefix.size() && std::string_view (name).starts_with (praefix);'
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'a6'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp'
        Zusage = 'audioGueltig weist eine riegelwidrige Samplerate ab (derselbe Ausdruck in der Grenze)'
        Alt = "            && nakama::wire::wireZahl (samplerate, verworfen)`n"; Neu = ''
        Ziel = 'EqCopSchemaTest'; Bein = 'B3c' }
    [pscustomobject]@{ Id = 'a7'; Klasse = 'a'; Datei = 'eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp'
        Zusage = 'Allokationsfehler in audioGueltig endet in std::terminate, nie als false verschluckt'
        Alt = "    catch (...)`n    {`n        std::terminate();`n    }"; Neu = "    catch (...)`n    {`n        return false;`n    }"
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'a8'; Klasse = 'a'; Datei = 'eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp'
        Zusage = 'Verstoss::operator< ordnet byteweise wie vorher (Reihenfolge der Verstossmenge)'
        Alt = 'if (p != 0) return p < 0;'; Neu = 'if (p != 0) return p > 0;'
        Ziel = 'EqCopSchemaTest'; Bein = 'B3c' }
    [pscustomobject]@{ Id = 'a9'; Klasse = 'a'; Datei = 'eq-copilot/plugin/vertrag/NakamaVertrag.cpp'
        Zusage = 'Verletzung::operator< ordnet byteweise wie vorher (Reihenfolge der Verletzungsmenge)'
        Alt = 'if (i != 0) return i < 0;'; Neu = 'if (i != 0) return i > 0;'
        Ziel = 'EqCopSchemaTest'; Bein = 'B3c' }
    [pscustomobject]@{ Id = 'a10'; Klasse = 'a'; Datei = 'eq-copilot/plugin/probe/PipeProbeMain.cpp'
        Zusage = 'main hat eine Ausnahmegrenze fuer jede Ausnahme (catch-all mit Meldung und Exit 70)'
        Alt = "catch (...)`n{`n    std::fputs (`"PROBE ABGEBROCHEN (unbekannte Ausnahme)\n`", stderr);`n    std::fflush (stderr);`n    return kExitAusnahme;`n}"; Neu = ''
        Ziel = 'EqCopPipeClientTest'; Bein = 'A4b' }
    [pscustomobject]@{ Id = 'd1'; Klasse = 'd'; Datei = 'eq-copilot/plugin/core/ipc/WireEnvelope.cpp'
        Zusage = 'bekanntes Encoding-Byte wird unveraendert zur Kodierung (angenommene Frames bleiben angenommen)'
        Alt = 'encodingBekannt ? static_cast<Kodierung> (encodingByte) : Kodierung::json;'
        Neu = 'encodingBekannt ? static_cast<Kodierung> (1 - encodingByte) : Kodierung::json;'
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'd2'; Klasse = 'd'; Datei = 'eq-copilot/plugin/core/ipc/WireEnvelope.cpp'
        Zusage = 'bekanntes Familien-Byte wird unveraendert zur Familie'
        Alt = 'familieBekannt  ? static_cast<Familie> (familieByte)    : Familie::p0;'
        Neu = 'familieBekannt  ? static_cast<Familie> ((familieByte + 1) % 3) : Familie::p0;'
        Ziel = 'EqCopIpcTest'; Bein = 'B10' }
    [pscustomobject]@{ Id = 'e1'; Klasse = 'e'; Datei = 'eq-copilot/plugin/dsp/DspKern.cpp'
        Zusage = 'die gemischte Haltezeit ist gerundet, nicht abgeschnitten (Holdzaehler sample-genau)'
        Alt = '(std::int64_t) std::llround ((double) a.holdSamples'; Neu = '(std::int64_t) std::trunc ((double) a.holdSamples'
        Ziel = 'EqCopDspGoldenTest'; Bein = 'B6' }
    [pscustomobject]@{ Id = 'f1'; Klasse = 'f'; Datei = 'eq-copilot/plugin/core/ipc/WireZahl.h'
        Zusage = 'der Exponent wird dezimal gelesen wie von atoi (Wire-Zahlen-Texte gleich)'
        Alt = 'std::strtol (wissenschaftlich.c_str() + ePos + 1, nullptr, 10)'; Neu = 'std::strtol (wissenschaftlich.c_str() + ePos + 1, nullptr, 8)'
        Ziel = 'EqCopSchemaTest'; Bein = 'B3c' }
    [pscustomobject]@{ Id = 'g1'; Klasse = 'g'; Datei = 'eq-copilot/plugin/probe/PipeProbeMain.cpp'
        Zusage = 'jede Ausgabezeile des Probewerkzeugs endet mit Umbruch UND flush'
        Alt = "<< `"...`" << '\n' << std::flush;"; Neu = "<< `"...`" << '\n';"
        Ziel = 'EqCopPipeClientTest'; Bein = 'A4b' }
    [pscustomobject]@{ Id = 'h1'; Klasse = 'h'; Datei = 'eq-copilot/plugin/vertrag/NakamaUtf8.h'
        Zusage = 'Zwei-Byte-Zweig: i steht nach dem Folgebyte genau dahinter'
        Alt = 'const auto b1 = p[i++];'; Neu = 'const auto b1 = p[i];'
        Ziel = 'EqCopPipeClientTest'; Bein = 'A4b' }
)

function Sha256Hex([byte[]] $b) { [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($b)) }

function Schreibe([string] $datei, [string[]] $zeilen) {
    Add-Content -LiteralPath (Join-Path $Wurzel $datei) -Value $zeilen -Encoding utf8NoBOM
}

function Baue([string] $ziel) {
    $aus = & $cmake --build eq-copilot/build --config Release --target $ziel 2>&1
    $code = $LASTEXITCODE
    $fehler = @($aus | Where-Object { "$_" -match '(?i)\berror\b|Fehler' } | Select-Object -First 10)
    [pscustomobject]@{ ExitCode = $code; Fehler = $fehler }
}

$gesamtOk = $true
$gebauteZiele = [System.Collections.Generic.List[string]]::new()
$gefahreneBeine = [System.Collections.Generic.List[string]]::new()
foreach ($m in $mutationen) {
    if ($Nur.Count -gt 0 -and $Nur -notcontains $m.Id) { continue }
    $pfad = Join-Path $Wurzel $m.Datei
    $rot = "docs/beweise/roh/NAK-289-rot-$($m.Klasse).txt"
    $orig = [IO.File]::ReadAllBytes($pfad)
    $shaVor = Sha256Hex $orig
    $bom = $orig.Length -ge 3 -and $orig[0] -eq 0xEF -and $orig[1] -eq 0xBB -and $orig[2] -eq 0xBF
    $text = [Text.UTF8Encoding]::new($false).GetString($orig, $(if ($bom) { 3 } else { 0 }), $orig.Length - $(if ($bom) { 3 } else { 0 }))
    $nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
    $alt = $m.Alt.Replace("`n", $nl)
    $neu = $m.Neu.Replace("`n", $nl)
    $treffer = [regex]::Matches($text, [regex]::Escape($alt)).Count
    Schreibe $rot @('', "=============== Rotbeweis $($m.Id) ($(Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))",
        "Zusage : $($m.Zusage)", "Datei  : $($m.Datei)", "Alt    : $($m.Alt.Replace("`n", '\n'))", "Neu    : $($m.Neu.Replace("`n", '\n'))",
        "SHA-256 vorher : $shaVor", "Treffer des alten Textes: $treffer")
    if ($treffer -ne 1) {
        Schreibe $rot @('ABBRUCH: der alte Text kommt nicht genau einmal vor - keine Mutation.')
        $gesamtOk = $false
        continue
    }
    $mutiert = $text.Replace($alt, $neu)
    $neuBytes = [Text.UTF8Encoding]::new($false).GetBytes($mutiert)
    if ($bom) { $neuBytes = [byte[]](0xEF, 0xBB, 0xBF) + $neuBytes }
    try {
        [IO.File]::WriteAllBytes($pfad, $neuBytes)
        (Get-Item -LiteralPath $pfad).LastWriteTime = Get-Date
        Schreibe $rot @("SHA-256 mutiert: $(Sha256Hex $neuBytes)")
        $bau = Baue $m.Ziel
        Schreibe $rot (@("Bau $($m.Ziel) mit Mutation: Exit $($bau.ExitCode)") + @($bau.Fehler | ForEach-Object { "  $_" }))
        if ($bau.ExitCode -ne 0) {
            Schreibe $rot @('BEFUND: Mutation baut nicht - kein Rotbeweis.')
            $gesamtOk = $false
        }
        else {
            & pwsh -NoProfile -File $beinSkript -Kuerzel $m.Bein -Ziel $exitDatei -Titel "Rotbeweis $($m.Id) mit Mutation" -Roh $rot | Out-Null
            $beinExit = $LASTEXITCODE
            Schreibe $rot @("Bein $($m.Bein) mit Mutation: Exit $beinExit (erwartet ungleich 0)")
            if ($beinExit -eq 0) { Schreibe $rot @('BEFUND: Bein bleibt mit Mutation gruen - misst die Zusage nicht.'); $gesamtOk = $false }
        }
    }
    finally {
        [IO.File]::WriteAllBytes($pfad, $orig)
        (Get-Item -LiteralPath $pfad).LastWriteTime = Get-Date
        $shaNach = Sha256Hex ([IO.File]::ReadAllBytes($pfad))
        $gleich = $shaNach -eq $shaVor
        Schreibe $rot @("SHA-256 nach Ruecknahme: $shaNach ($(if ($gleich) { 'bytegleich' } else { 'NICHT BYTEGLEICH' }))")
        if (-not $gleich) { $gesamtOk = $false }
    }
    if (-not $gebauteZiele.Contains($m.Ziel)) { $gebauteZiele.Add($m.Ziel) }
    if (-not $gefahreneBeine.Contains("$($m.Bein)|$($m.Klasse)")) { $gefahreneBeine.Add("$($m.Bein)|$($m.Klasse)") }
}

# Nach allen Ruecknahmen: Ziele neu bauen, Beine gruen fahren.
foreach ($z in $gebauteZiele) {
    $bau = Baue $z
    Schreibe 'docs/beweise/roh/NAK-289-rot-a.txt' @("Schlussbau $z nach allen Ruecknahmen: Exit $($bau.ExitCode)")
    if ($bau.ExitCode -ne 0) { $gesamtOk = $false }
}
foreach ($eintrag in $gefahreneBeine) {
    $bein, $klasse = $eintrag.Split('|')
    $rot = "docs/beweise/roh/NAK-289-rot-$klasse.txt"
    & pwsh -NoProfile -File $beinSkript -Kuerzel $bein -Ziel $exitDatei -Titel "Rotbeweise Klasse $klasse nach Ruecknahme" -Roh $rot | Out-Null
    $beinExit = $LASTEXITCODE
    Schreibe $rot @("Bein $bein nach Ruecknahme: Exit $beinExit (erwartet 0)")
    if ($beinExit -ne 0) { $gesamtOk = $false }
}
exit $(if ($gesamtOk) { 0 } else { 2 })
