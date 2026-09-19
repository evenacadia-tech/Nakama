#Requires -Version 7.4
<#
NAK-309 Etappe 4 - Selbstaudit (Auftrag Schritt 6): der Urteilsblock von tools/beweise.ps1 unter NOT RUN
zusammen mit ROT, mit einer fehlenden Voraussetzung und mit VERALTET. Ergebnis:
docs/beweise/roh/NAK-309-etappe4-urteilskombi.txt.

Je Fall eine Kopie des Runners im Temp-Ordner mit genau diesen Ersetzungen (jede muss genau einmal treffen):
  $Wurzel             -> der Workspace-Root (die Kopie liegt nicht unter tools/)
  beweise-roh.ps1     -> absoluter Pfad
  $rohVerzeichnis     -> Temp-Ordner (keine Rohausgabe im Repo)
  nach "$nichtGelaufen = 0" eine eigene $kanon-Liste. Baustand, Frischepruefung und Urteilsblock laufen
  unveraendert; die Baustandpruefung davor sieht den vollen Kanon.
Beine der Faelle:
  NR    cargo test --test store_crash_matrix store_weist_reparse_punkt_im_pfad_ab, Meldeweg wie A4; mit
        NAKAMA_TEST_JUNCTION_VERWEIGERN=1 in der Umgebung des Runnerprozesses meldet der Test NOT RUN.
  OK    py pruefe_beweisrunner.py --nicht-gelaufen <leerer Ordner>      -> Exit 0, [OK]
  ROT   py pruefe_beweisrunner.py --nicht-gelaufen <fehlender Ordner>   -> Exit 2, [ROT]
  FEHLT py pruefe_beweisrunner.py --nicht-gelaufen <Ordner mit Marke>   -> Exit 3: ein Python-Bein mit Exit 3
        nimmt im Runner denselben Zweig wie A9 ohne flatc ([FEHLT] "Voraussetzung fehlt (Exit 3)").
Kein -Bauen: VERALTET ist der Baustand, den der Lauf vorfindet (er steht je Fall in der Rohdatei).
Der Schalter steht nur in der Umgebung des Runnerprozesses des Falls (Start-Process -Environment).
#>
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$Temp = Join-Path ([IO.Path]::GetTempPath()) 'nakama-nak309-e4-kombi'
[void](New-Item -ItemType Directory -Force -Path $Temp)
$Utf8 = New-Object System.Text.UTF8Encoding($false)
$WerkzeugSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $PSCommandPath).Hash
$Runner = Join-Path $Repo 'tools\beweise.ps1'
$RunnerSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $Runner).Hash
$Pwsh = (Get-Command pwsh).Source

$leer = Join-Path $Temp 'leer'
$fehlt = Join-Path $Temp 'gibt-es-nicht'
$marke = Join-Path $Temp 'marke'
foreach ($o in @($leer, $marke)) { [void](New-Item -ItemType Directory -Force -Path $o) }
foreach ($f in @(Get-ChildItem -LiteralPath $leer -Force)) { [IO.File]::Delete($f.FullName) }
if (Test-Path -LiteralPath $fehlt) { [IO.Directory]::Delete($fehlt, $true) }
[IO.File]::WriteAllText((Join-Path $marke 'kombi_voraussetzung.nicht-gelaufen'), "kombi_voraussetzung: Stellvertreter fuer ein Bein mit Exit 3`n", $Utf8)

$Bein = @{
    NR    = "[pscustomobject]@{ Kuerzel='NR'; Name='broker'; Art='cargo'; Argumente=@('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never', '--test', 'store_crash_matrix', 'store_weist_reparse_punkt_im_pfad_ab'); AbPhase='jetzt'; Meldeweg=`$true; Behauptung='Kombi NR: store_weist_reparse_punkt_im_pfad_ab mit Meldeweg' }"
    OK    = "[pscustomobject]@{ Kuerzel='OK'; Name='pruefe_beweisrunner.py'; Art='python'; Argumente=@('--nicht-gelaufen', '$leer'); AbPhase='jetzt'; Behauptung='Kombi OK: leerer Meldeordner' }"
    ROT   = "[pscustomobject]@{ Kuerzel='ROT'; Name='pruefe_beweisrunner.py'; Art='python'; Argumente=@('--nicht-gelaufen', '$fehlt'); AbPhase='jetzt'; Behauptung='Kombi ROT: Meldeordner fehlt' }"
    FEHLT = "[pscustomobject]@{ Kuerzel='FEHLT'; Name='pruefe_beweisrunner.py'; Art='python'; Argumente=@('--nicht-gelaufen', '$marke'); AbPhase='jetzt'; Behauptung='Kombi FEHLT: Python-Bein mit Exit 3' }"
}

function Ersetze([string]$text, [string]$alt, [string]$neu) {
    $n = ([regex]::Matches($text, [regex]::Escape($alt))).Count
    if ($n -ne 1) { throw "Ersetzung trifft $n-mal statt genau einmal: $alt" }
    return $text.Replace($alt, $neu)
}

function Kopie([string]$fall, [string[]]$beine) {
    $t = [IO.File]::ReadAllText($Runner, $Utf8)
    $t = Ersetze $t "`$Wurzel = (Resolve-Path (Join-Path `$PSScriptRoot '..')).Path" "`$Wurzel = '$Repo'"
    $t = Ersetze $t ". (Join-Path `$PSScriptRoot 'beweise-roh.ps1')" ". '$(Join-Path $Repo 'tools\beweise-roh.ps1')'"
    $t = Ersetze $t "`$rohVerzeichnis = Join-Path `$Wurzel 'docs\beweise\roh'" "`$rohVerzeichnis = '$Temp'"
    $liste = '$kanon = @(' + ((@($beine | ForEach-Object { $Bein[$_] })) -join ', ') + ')'
    $t = Ersetze $t "`$nichtGelaufen = 0" ("`$nichtGelaufen = 0`r`n# KOMBI $fall`r`n" + $liste)
    $pfad = Join-Path $Temp "beweise-kombi-$fall.ps1"
    [IO.File]::WriteAllText($pfad, $t, $Utf8)
    return $pfad
}

function Meldeordner { return @(Get-ChildItem -LiteralPath ([IO.Path]::GetTempPath()) -Directory -Filter 'nakama-nicht-gelaufen-*' -ErrorAction SilentlyContinue) }

$z = New-Object System.Collections.Generic.List[string]
$kopf = (& git -C $Repo rev-parse --short=8 HEAD).Trim()
$z.Add('NAK-309 Etappe 4 - Urteilskombinationen des Runners (Selbstaudit, Auftrag Schritt 6)')
$z.Add(("Stand:  {0:yyyy-MM-dd HH:mm}, HEAD {1}; tools/beweise.ps1 SHA-256 {2}" -f (Get-Date), $kopf, $RunnerSha))
$z.Add("Werkzeug: docs/beweise/roh/NAK-309-etappe4-urteilskombi.ps1 (SHA-256 $WerkzeugSha)")
$z.Add("Meldeordner im Temp-Ordner vor dem ersten Fall: $((Meldeordner).Count)")
$z.Add('')

$faelle = @(
    @{ Fall = 'K1'; Beine = @('NR', 'ROT', 'OK'); Schalter = $true;  Exit = 2; Muster = '^ROT - 1 von 3 Kanon-Laeufen fehlgeschlagen.* \| 1 Bein\(e\) NOT RUN, siehe Uebersicht'; Satz = 'NOT RUN mit ROT: ROT gewinnt (Exit 2), der Nachsatz nennt NOT RUN' }
    @{ Fall = 'K2'; Beine = @('NR', 'FEHLT', 'OK'); Schalter = $true;  Exit = 3; Muster = '^UNVOLLSTAENDIG - 1 gruen, 2 Voraussetzung\(en\) fehlen.* \| 1 Bein\(e\) NOT RUN, siehe Uebersicht'; Satz = 'NOT RUN mit einer fehlenden Voraussetzung (Exit 3 eines Python-Beins): Exit 3, beide zaehlen als Voraussetzung, NOT RUN nie gruen' }
    @{ Fall = 'K3'; Beine = @('NR', 'OK'); Schalter = $true;  Exit = 3; Muster = '^UNVOLLSTAENDIG - 1 gruen, 1 Voraussetzung\(en\) fehlen.* \| 1 Bein\(e\) NOT RUN, siehe Uebersicht'; Satz = 'NOT RUN bei veraltetem Baustand: Exit 3 vor 4' }
    @{ Fall = 'K4'; Beine = @('OK'); Schalter = $false; Exit = 4; Muster = '^NICHT BEGLAUBIGT - 1/1 gruen, aber Pruefbinaries oder gemessene Ziele sind aelter als ihre Quellen'; Satz = 'Gegenprobe zu K3: derselbe Baustand ohne NOT RUN ist NICHT BEGLAUBIGT (Exit 4) - VERALTET liegt wirklich vor' }
)
$fehler = 0
foreach ($f in $faelle) {
    $kopie = Kopie $f.Fall $f.Beine
    $ziel = Join-Path $Temp "kombi-$($f.Fall).md"
    if (Test-Path -LiteralPath $ziel) { [IO.File]::Delete($ziel) }
    foreach ($alt in @(Get-ChildItem -LiteralPath $Temp -File -Filter "kombi-$($f.Fall)-*.md")) { [IO.File]::Delete($alt.FullName) }
    $aus = Join-Path $Temp "kombi-$($f.Fall).out"
    $sp = @{ FilePath = $Pwsh; ArgumentList = "-NoProfile -File `"$kopie`" -Ziel `"$ziel`""; WorkingDirectory = $Repo; NoNewWindow = $true; PassThru = $true; RedirectStandardOutput = $aus; RedirectStandardError = "$aus.err" }
    if ($f.Schalter) { $sp.Environment = @{ NAKAMA_TEST_JUNCTION_VERWEIGERN = '1' } }
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process @sp
    $null = $p.Handle
    $p.WaitForExit()
    $uhr.Stop()
    $manifest = if (Test-Path -LiteralPath $ziel) { [IO.File]::ReadAllText($ziel, $Utf8) } else { '' }
    $urteil = if ($manifest -match '\*\*Urteil:\*\* (?<u>.*?) \| \*\*Exitcode:\*\* (?<e>\d+)') { $Matches['u'] } else { '(kein Urteil im Manifest)' }
    $ok = ($p.ExitCode -eq $f.Exit) -and ($urteil -match $f.Muster)
    if (-not $ok) { $fehler++ }
    $z.Add("--- $($f.Fall): $($f.Satz) ---")
    $z.Add("Beine: $($f.Beine -join ', '); Schalter NAKAMA_TEST_JUNCTION_VERWEIGERN=1 nur fuer den Runnerprozess: $(if ($f.Schalter) { 'ja' } else { 'nein' })")
    $z.Add("Aufruf: pwsh -NoProfile -File <Temp>\beweise-kombi-$($f.Fall).ps1 -Ziel <Temp>\kombi-$($f.Fall).md")
    $z.Add("Exitcode: $($p.ExitCode) (erwartet $($f.Exit)), $([math]::Round($uhr.Elapsed.TotalSeconds, 1)) s")
    $z.Add("Urteil:   $urteil")
    foreach ($zeile in @($manifest -split "`r?`n" | Where-Object { $_ -match '^\| (NR|OK|ROT|FEHLT) \|' })) {
        $zellen = $zeile -split '\s\|\s'
        $z.Add(("  Bein {0}: {1}" -f $zellen[0].Trim('| '), $zellen[3]))
    }
    $rohText = (@(Get-ChildItem -LiteralPath $Temp -File -Filter "kombi-$($f.Fall)-*.md" | Sort-Object LastWriteTime | Select-Object -Last 1) | ForEach-Object { [IO.File]::ReadAllText($_.FullName, $Utf8) }) -join ''
    $veraltet = @($rohText -split "`r?`n" | Where-Object { $_ -match '^\|[^|]+\|[^|]+\|[^|]+\|\s*VERALTET\b' })
    $hinweis = @($manifest -split "`r?`n" | Where-Object { $_ -match '^> Baustand:' }) | Select-Object -First 1
    $z.Add("Manifestzeile: $(if ($hinweis) { $hinweis } else { '(kein Baustandhinweis)' })")
    $z.Add("Baustandtabelle der Rohausgabe: $($veraltet.Count) Zeile(n) VERALTET$(if ($veraltet.Count) { ', erste: ' + $veraltet[0].Trim() })")
    $z.Add("Erwartung erfuellt: $(if ($ok) { 'JA' } else { 'NEIN' })")
    $z.Add("Meldeordner im Temp-Ordner nach dem Fall: $((Meldeordner).Count)")
    $z.Add('')
}
$z.Add(("Umgebung dieses Prozesses: NAKAMA_TEST_JUNCTION_VERWEIGERN={0}, NAKAMA_NICHT_GELAUFEN={1}; Benutzer: {2}; Maschine: {3}" -f `
    $(if ($null -eq $env:NAKAMA_TEST_JUNCTION_VERWEIGERN) { '(nicht gesetzt)' } else { $env:NAKAMA_TEST_JUNCTION_VERWEIGERN }),
    $(if ($null -eq $env:NAKAMA_NICHT_GELAUFEN) { '(nicht gesetzt)' } else { $env:NAKAMA_NICHT_GELAUFEN }),
    $(if ($null -eq [Environment]::GetEnvironmentVariable('NAKAMA_TEST_JUNCTION_VERWEIGERN', 'User')) { '(nicht gesetzt)' } else { 'GESETZT' }),
    $(if ($null -eq [Environment]::GetEnvironmentVariable('NAKAMA_TEST_JUNCTION_VERWEIGERN', 'Machine')) { '(nicht gesetzt)' } else { 'GESETZT' })))
$z.Add("Faelle mit erfuellter Erwartung: $($faelle.Count - $fehler) von $($faelle.Count)")
$roh = Join-Path $Repo 'docs\beweise\roh\NAK-309-etappe4-urteilskombi.txt'
[IO.File]::WriteAllText($roh, [string]::Join("`n", $z) + "`n", $Utf8)
Write-Host "geschrieben: docs/beweise/roh/NAK-309-etappe4-urteilskombi.txt ($($faelle.Count - $fehler) von $($faelle.Count))"
exit $(if ($fehler -gt 0) { 1 } else { 0 })
