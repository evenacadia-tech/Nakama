<#
NAK-286 Nacharbeit 1 der Abschlusspruefung - Weg R2 nach M-64 (Regel P-21, Manifest docs/beweise/NAK-286.md §44.3,
§45): Rotlaeufe vor der Aenderung und Rotbeweise nach dem Bau (§5.1, P-20). Muster: NAK-286-rotskript-etappe5.ps1.

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-abschluss-nacharbeit1.ps1 -Art vorher   Rotlaeufe vor der Aenderung
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-abschluss-nacharbeit1.ps1 -Art nach     Rotbeweise nach dem Bau
  ... -Nur 'M-64 (c)','M-64 (d)'   nur diese Eintraege        ... -NurPruefen   nur Traegertexte zaehlen

Je Eintrag, in dieser Reihenfolge (§5.1; PowerShell und Python ohne den LastWriteTime-Schritt):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot-nach1).
  2. Mutation genau an der Traegerzeile, jeder Alttext genau einmal (P-20: haelt eine redundante Schranke die
     Zusage, gehoert sie zur Mutation). Ein Rotlauf vor der Aenderung faehrt den neuen Fall gegen den noch
     unveraenderten Traeger und mutiert nichts.
  3. Das Bein einzeln, mit Obergrenze: A35 als einzelner Selbsttestfall (laufzeit.ps1 -Selbsttest -Nur,
     szenario.py --selbsttest --nur, nulltest.py --selbsttest --nur). Nach Ablauf der Obergrenze wird der Prozessbaum
     beendet (taskkill /T /F); das Ende an der Obergrenze steht in der Rohdatei. Erwartung "rot": Exit ungleich 0 und
     mindestens die verlangte Zahl Zeilen, die den Traeger nennen (Muster des Traegers, nie der Meldung).
  4. Bytegleiche Ruecknahme aus den gesicherten Bytes (Schreiben mit Wiederholung), SHA-256 gleich Schritt 1,
     derselbe Lauf gruen.
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-64-nach1.txt, je Eintrag ein angehaengter Abschnitt. Der Lauf startet kein
FL, installiert nichts und rendert nichts: jedes Bein laeuft gegen Attrappen, nie mit einem echten Render.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen, [ValidateSet('vorher', 'nach')] [string] $Art = 'nach',
      [int] $ObergrenzeSekunden = 240)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$utf8 = New-Object System.Text.UTF8Encoding($false)

$tmp = Join-Path $env:TEMP 'nak286-rot-nach1'
New-Item -ItemType Directory -Force $tmp | Out-Null
$env:PYTHONIOENCODING = 'utf-8'
$env:PYTHONDONTWRITEBYTECODE = '1'

$LZ = 'tools/fl/laufzeit.ps1'
$SZ = 'tools/fl/szenario.py'
$NT = 'tools/fl/nulltest.py'
$NH = 'docs/gesundheit/szenarien/nulltest-host.json'
$MCP = 'C:\Users\phili\Projekte\fl-studio-mcp'
# Name der Rohdatei; eigener Bezeichner, weil PowerShell $ROH und $roh nicht unterscheidet (erster Rotlauf 15.09.2026 nach
# dem Eintrag M-64 (c) an der verdoppelten Pfadangabe abgebrochen).
$ROHDATEI = 'NAK-286-rot-M-64-nach1.txt'

function Pfad ([string] $d) { if ([IO.Path]::IsPathRooted($d)) { $d } else { Join-Path $wurzel $d } }
# Schreiben mit Wiederholung: ein gerade endender Prozess haelt eine frisch geschriebene Quelle kurz als
# abgebildeten Bereich offen (Rotskriptlauf der Etappe 4, 15.09.2026, §37.9 Punkt 9).
function Schreibe ([string] $pfad, [byte[]] $bytes)
{
    for ($versuch = 1; ; $versuch++)
    {
        try { [IO.File]::WriteAllBytes($pfad, $bytes); return }
        catch { if ($versuch -ge 100) { throw }; Start-Sleep -Milliseconds 200 }
    }
}
function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Eintrag ([string] $art, [string] $zeile, [string] $teil, [string] $beschreibung, [string] $test, [string] $traeger,
                  [string] $erwartung, [string[]] $quellen, [object[]] $ersetzungen, [int] $mindestens = 1)
{
    [pscustomobject]@{ Art = $art; Zeile = $zeile; Teil = $teil; Beschreibung = $beschreibung; Test = $test; Traeger = $traeger
                       Erwartung = $erwartung; Quellen = $quellen; Ersetzungen = $ersetzungen; Mindestens = $mindestens }
}

$eintraege = New-Object System.Collections.Generic.List[object]

# ================================================================ Rotlaeufe vor der Aenderung (P-21 (g))
# nulltest.py: die neuen Faelle gegen den unveraenderten Stand - Render gegen Quelle mit umgekehrter Bewertung und
# Render gegen Render gibt es dort nicht (nulltest.py kennt nur R1, vergleiche und EXIT_JE_URTEIL).
$eintraege.Add((Eintrag 'vorher' 'M-64' '(c)' 'neuer Fall verarbeitung_ein_umgekehrt gegen das unveraenderte nulltest.py (nur Render gegen Quelle, Bewertung R1)' 'nt:verarbeitung_ein_umgekehrt' '\[ROT\] M-64 verarbeitung_ein_umgekehrt:.*fahre_referenz' 'rot' @($NT) @()))
$eintraege.Add((Eintrag 'vorher' 'M-64' '(d)' 'neuer Fall ohne_slots_render_gegen_render gegen das unveraenderte nulltest.py (kein Render-gegen-Render-Vergleich)' 'nt:ohne_slots_render_gegen_render' '\[ROT\] M-64 ohne_slots_render_gegen_render:.*fahre_referenz' 'rot' @($NT) @()))

# szenario.py: der neue Fall mit Renderstatus und Render gegen das unveraenderte lokal.nulltest (Existenz im Repo-Ordner,
# bei vorhandenem Projekt "Weg R2 nicht gebaut").
$eintraege.Add((Eintrag 'vorher' 'M-64' '(b)' 'neuer Fall referenzschritte_aus_renderstatus gegen das unveraenderte lokal.nulltest (Attrappe mit vorhandenem Referenzprojekt)' 'sz:referenzschritte_aus_renderstatus' '\[ROT\] M-64 referenzschritte_aus_renderstatus:.*verarbeitung_ein mit Renderstatus und Render: Exit 5 statt 0.*Weg R2 nicht gebaut' 'rot' @($SZ) @()))

# laufzeit.ps1: der neue Fall mit der Szenariodatei des Repos und den Projekten der Karte gegen den unveraenderten Runner
# (nur der Render des Diagnoseprojekts).
$eintraege.Add((Eintrag 'vorher' 'M-64' '(a)' 'neuer Fall referenzrender_vor_fl_start gegen den unveraenderten Runner (ein Render statt drei)' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*Renders vor dem FL-Start: 1 statt 3' 'rot' @($LZ, $NH) @()))

# ================================================================ Rotbeweise nach dem Bau
# P-21 (c) an der Bewertung von verarbeitung_ein: 0 Abweichungen gilt als gemessen - der blinde Vergleich wird gruen.
$eintraege.Add((Eintrag 'nach' 'M-64' '(c)' 'verarbeitung_ein: 0 Abweichungen (BITIDENTISCH, VERSATZ) als GEMESSEN bewertet' 'nt:verarbeitung_ein_umgekehrt' '\[ROT\] M-64 verarbeitung_ein_umgekehrt:.*0 Abweichungen \(BITIDENTISCH, v 0\): Urteil GEMESSEN' 'rot' @() @(
    (Ersetzung $NT @'
    elif abweichungen > 0:
'@ @'
    elif abweichungen >= 0:  # ROTBEWEIS M-64 (c): 0 Abweichungen gilt als gemessen
'@))))

# P-21 (d) am SHA-256-Vergleich von ohne_slots: ungleiche Datenbereiche gelten als gleich.
$eintraege.Add((Eintrag 'nach' 'M-64' '(d)' 'ohne_slots: SHA-256 des Referenzrenders nicht gegen sha256_render_bereich des Auslieferungsrenders verglichen' 'nt:ohne_slots_render_gegen_render' '\[ROT\] M-64 ohne_slots_render_gegen_render:.*ein gekipptes LSB an Frame 98765: Urteil GEMESSEN Exit 0 statt VERFEHLT 4' 'rot' @() @(
    (Ersetzung $NT @'
    gleich = sha == a["sha256_render_bereich"]
'@ @'
    gleich = True  # ROTBEWEIS M-64 (d): SHA-256 nicht verglichen
'@))))

# P-21 (f) an der Rohzeile: Projektname, SHA-256 des Projekts und Renderdauer fehlen.
$eintraege.Add((Eintrag 'nach' 'M-64' '(f)' 'Rohzeile eines Referenzschritts ohne Projektname, SHA-256 des Projekts und Renderdauer' 'nt:verarbeitung_ein_umgekehrt' "\[ROT\] M-64 verarbeitung_ein_umgekehrt:.*Rohzeile verarbeitung_ein ohne 'Projekt band\.flp'" 'rot' @() @(
    (Ersetzung $NT @'
        teile += [f"Projekt {e.get('projekt')}", f"SHA-256 Projekt {e.get('sha256_projekt')}", f"Renderdauer {e.get('renderdauer_s')} s"]
'@ @'
        pass  # ROTBEWEIS M-64 (f): Projektname, SHA-256 des Projekts und Renderdauer nicht in der Rohzeile
'@))))

# P-21 (b) am Lesen des Renderstatus: wieder die Existenz einer Datei im Repo-Ordner statt des Renderstatus.
$eintraege.Add((Eintrag 'nach' 'M-64' '(b) Renderstatus' 'lokal.nulltest prueft wieder die Existenz des Projekts im Repo-Ordner statt den Renderstatus zu lesen' 'sz:referenzschritte_aus_renderstatus' '\[ROT\] M-64 referenzschritte_aus_renderstatus:.*ohne_slots mit Renderstatus und Render \(kein Projekt im Repo-Ordner\)' 'rot' @() @(
    (Ersetzung $SZ @'
    ordner = lauf.render_ordner / REFERENZ_ORDNER / projekt
'@ @'
    if not lauf.umg.existiert(REPO / "eq-copilot" / "fixtures" / "fl" / projekt):  # ROTBEWEIS M-64 (b): Existenz im Repo-Ordner statt Renderstatus
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: Referenzprojekt fehlt (Karte U43, K-286-1)", [f"- erwartet: `{projekt}`"])
    ordner = lauf.render_ordner / REFERENZ_ORDNER / projekt
'@))))

# P-21 (b), P-3: fehlendes Projekt still - ohne Szenario-Exit 5.
$eintraege.Add((Eintrag 'nach' 'M-64' '(b) Exit 5' 'ohne Render endet der Referenzschritt mit Exit 0 statt 5' 'sz:referenzschritte_aus_renderstatus' '\[ROT\] M-64 referenzschritte_aus_renderstatus:.*fehlendes Referenzprojekt: Exit 0 statt 5' 'rot' @() @(
    (Ersetzung $SZ @'
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: {grund}",
'@ @'
        return (EXIT_OK, f"VORAUSSETZUNG {vergleich}: {grund}",  # ROTBEWEIS M-64 (b): ohne Render Exit 0 statt 5
'@))))

# P-21 (b), P-3: fehlendes Projekt still - ohne Rohzeile.
$eintraege.Add((Eintrag 'nach' 'M-64' '(b) Rohzeile' 'ohne Render endet der Referenzschritt ohne Rohzeile' 'sz:referenzschritte_aus_renderstatus' '\[ROT\] M-64 referenzschritte_aus_renderstatus:.*fehlendes Referenzprojekt \(verarbeitung_ein\) still: keine Rohzeile' 'rot' @() @(
    (Ersetzung $SZ @'
                [f"- Renderstatus `{status_pfad}`: projekt `{status.get('projekt')}`, grund {_json(grund)} - kein Render "
                 f"dieses Zustands (M-64), nie still"])
'@ @'
                [])  # ROTBEWEIS M-64 (b): ohne Render ohne Rohzeile
'@))))

# P-21 (e), M-65: das Ergebnis eines Referenzschritts landet in der ergebnis.json des Auslieferungsrenders.
$eintraege.Add((Eintrag 'nach' 'M-64' '(e)' 'Ergebnis des Referenzschritts in die ergebnis.json des Auslieferungsrenders (die der Runner fuer den Rueckweg liest)' 'sz:referenzschritte_aus_renderstatus' '\[ROT\] M-64 referenzschritte_aus_renderstatus:.*ergebnis\.json des Auslieferungsrenders durch einen Referenzschritt veraendert' 'rot' @() @(
    (Ersetzung $SZ @'
    ergebnis_pfad = ordner / "ergebnis.json"
'@ @'
    ergebnis_pfad = lauf.render_ordner / "ergebnis.json"  # ROTBEWEIS M-64 (e): Ergebnis in die ergebnis.json des Auslieferungsrenders
'@))))

# P-21 (a) an der Stelle im Ablauf: die Referenzrender laufen nach dem FL-Start.
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) vor dem FL-Start' 'Rendere-Referenzprojekte nach Warte-Ping verschoben (Referenzrender nach dem FL-Start)' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*Render 2 Nakama-Diagnose-Verarbeitung\.flp nach dem FL-Start' 'rot' @() @(
    (Ersetzung $LZ @'
            Rendere-Referenzprojekte $liste
            Stelle-LoopMidi-Sicher
'@ @'
            Stelle-LoopMidi-Sicher
'@),
    (Ersetzung $LZ @'
            $K.Ping = Warte-Ping
'@ @'
            $K.Ping = Warte-Ping
            Rendere-Referenzprojekte $liste # ROTBEWEIS M-64 (a): Referenzrender nach dem FL-Start
'@))))

# P-21 (a), F-26: der Referenzrender schreibt in den Ordner und die Dateien des Auslieferungsrenders.
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) eigene Dateien' 'Referenzrender in den Ordner render\ statt render\referenz\<Name>\ (render.json des Auslieferungsrenders ueberschrieben)' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*render\.json des Auslieferungsrenders nennt projekt' 'rot' @() @(
    (Ersetzung $LZ @'
        $ordner = Join-Path $K.RenderOrdner "referenz\$projektname"
'@ @'
        $ordner = $K.RenderOrdner  # ROTBEWEIS M-64 (a): Referenzrender in den Ordner und die Dateien des Auslieferungsrenders
'@))))

# P-21 (a) an der Quelle des Renders: das Referenzprojekt aus dem Repo-Ordner statt aus der Arbeitskopie.
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) Arbeitskopie' 'Referenzrender aus dem Ordner des Diagnoseprojekts im Repo statt aus der Arbeitskopie' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*Render 2 Nakama-Diagnose-Verarbeitung\.flp nicht aus der Arbeitskopie' 'rot' @() @(
    (Ersetzung $LZ @'
        $kopie = Join-Path $K.ProjektOrdner $projektname
'@ @'
        $kopie = Join-Path ([IO.Path]::GetDirectoryName($K.Projekt)) $projektname  # ROTBEWEIS M-64 (a): Render aus dem Repo-Ordner statt aus der Arbeitskopie
'@))))

# P-21 (a) am fehlenden Projekt: kein Renderstatus.
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) Renderstatus fehlt' 'fehlendes Referenzprojekt ohne Renderstatus' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*fehlendes Referenzprojekt Nakama-Diagnose-Verarbeitung\.flp: Renderstatus fehlt' 'rot' @() @(
    (Ersetzung $LZ @'
        & $script:U.SchreibeText $statusDatei ($fehlend | ConvertTo-Json -Depth 4)
'@ @'
        # ROTBEWEIS M-64 (a): kein Renderstatus fuer ein fehlendes Referenzprojekt
'@))))

# P-21 (a) am fehlenden Projekt: keine Protokollzeile "kein Render".
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) Rohzeile fehlt' 'fehlendes Referenzprojekt ohne Protokollzeile' 'ps:referenzrender_vor_fl_start' "\[ROT\] M-64 referenzrender_vor_fl_start:.*fehlendes Referenzprojekt Nakama-Diagnose-Verarbeitung\.flp: keine Protokollzeile 'kein Render'" 'rot' @() @(
    (Ersetzung $LZ @'
        Log "Referenzrender ${projektname}: kein Render - $($script:ReferenzFehlt); Renderstatus $statusDatei"
'@ @'
        # ROTBEWEIS M-64 (a): keine Protokollzeile fuer ein fehlendes Referenzprojekt
'@))))

# P-21 (a): die Reihenfolge der Renders steht nicht im Protokoll.
$eintraege.Add((Eintrag 'nach' 'M-64' '(a) Renderfolge' 'Renderfolge vor dem FL-Start nicht protokolliert' 'ps:referenzrender_vor_fl_start' '\[ROT\] M-64 referenzrender_vor_fl_start:.*Protokoll ohne Renderfolge vor dem FL-Start' 'rot' @() @(
    (Ersetzung $LZ @'
            Nenne-Renderfolge $liste
'@ @'
            # ROTBEWEIS M-64 (a): Renderfolge nicht im Protokoll
'@))))

# ================================================================ Lauf
function Fahre ([string] $test, [string] $ausgabe)
{
    $testart, $rest = $test -split ':', 2
    $befehl = switch ($testart)
    {
        'ps' { @('pwsh', '-NoProfile', '-File', $LZ, '-Selbsttest', '-Nur', $rest) }
        'sz' { @('py', '-3.13', $SZ, '--selbsttest', '--nur', $rest) }
        'nt' { @('py', '-3.13', $NT, '--selbsttest', '--nur', $rest) }
        default { throw "unbekannte Testart $testart" }
    }
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = (Get-Command $befehl[0] -CommandType Application | Select-Object -First 1).Source
    foreach ($a in @($befehl | Select-Object -Skip 1)) { $info.ArgumentList.Add([string]$a) }
    $info.WorkingDirectory = $wurzel
    $info.UseShellExecute = $false
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.StandardOutputEncoding = $utf8
    $info.StandardErrorEncoding = $utf8
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $p = [Diagnostics.Process]::Start($info)
    $aus = $p.StandardOutput.ReadToEndAsync()
    $fehler = $p.StandardError.ReadToEndAsync()
    $obergrenze = -not $p.WaitForExit($ObergrenzeSekunden * 1000)
    if ($obergrenze)
    {
        & taskkill /PID $p.Id /T /F 2>&1 | Out-Null
        $null = $p.WaitForExit(30000)
    }
    $null = $aus.Wait(30000); $null = $fehler.Wait(30000)
    $uhr.Stop()
    $text = ''
    if ($aus.IsCompleted) { $text += $aus.Result }
    if ($fehler.IsCompleted) { $text += $fehler.Result }
    if ($obergrenze) { $text += "`nOBERGRENZE: Lauf nach $ObergrenzeSekunden s nicht beendet, Prozessbaum beendet (taskkill /T /F)`n" }
    [IO.File]::WriteAllText($ausgabe, $text, $utf8)
    $exit = if ($p.HasExited) { $p.ExitCode } else { -1 }
    return [pscustomobject]@{ Exit = $exit; Befehl = ($befehl -join ' '); Dauer = [math]::Round($uhr.Elapsed.TotalSeconds, 1); Obergrenze = $obergrenze }
}

# Mit `pwsh -File` kommt `-Nur a,b` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($eintraege | Where-Object { $_.Art -eq $Art -and ($Nur.Count -eq 0 -or $Nur -contains $_.Zeile -or $Nur -contains ($_.Zeile + ' ' + $_.Teil).Trim()) })
$zusammenfassung = New-Object System.Collections.Generic.List[string]
$nakamaHead = (& git -C $wurzel rev-parse HEAD)
$mcpHead = (& git -C $MCP rev-parse HEAD)
$artWort = @{ vorher = 'Rotlauf vor der Aenderung'; nach = 'Rotbeweis' }[$Art]
$geprueft = 0

foreach ($m in $auswahl)
{
    $name = ($m.Zeile + ' ' + $m.Teil).Trim()
    $dateien = @(@($m.Quellen) + @($m.Ersetzungen | ForEach-Object { $_.Datei }) | Select-Object -Unique)
    $orig = @{}; $shaVor = @{}; $texte = @{}
    foreach ($d in $dateien)
    {
        $orig[$d] = [IO.File]::ReadAllBytes((Pfad $d))
        $shaVor[$d] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Pfad $d)).Hash
        [IO.File]::WriteAllBytes((Join-Path $tmp ((Split-Path $d -Leaf) + '.orig')), $orig[$d])
        $texte[$d] = $utf8.GetString($orig[$d])
    }
    $diff = New-Object System.Collections.Generic.List[string]
    foreach ($e in @($m.Ersetzungen))
    {
        $t = $texte[$e.Datei]
        $nl = if ($t.Contains("`r`n")) { "`r`n" } else { "`n" }
        $alt = $e.Alt -replace "`r?`n", $nl
        $neu = $e.Neu -replace "`r?`n", $nl
        $n = [regex]::Matches($t, [regex]::Escape($alt)).Count
        if ($n -ne 1) { throw "$name - Traegertext in $($e.Datei) $n-mal statt genau einmal: $($e.Alt)" }
        $i = $t.IndexOf($alt, [StringComparison]::Ordinal)
        $texte[$e.Datei] = $t.Substring(0, $i) + $neu + $t.Substring($i + $alt.Length)
        $diff.Add("Ersetzung in $($e.Datei):")
        foreach ($z in ($e.Alt -split "`r?`n")) { $diff.Add("  - $z") }
        foreach ($z in ($e.Neu -split "`r?`n")) { $diff.Add("  + $z") }
    }
    if ($NurPruefen) { $geprueft++; Write-Output "PRUEFUNG OK $Art $name"; continue }

    $mutiert = @($m.Ersetzungen).Count -gt 0
    $shaMut = @{}
    $rot = $null; $rotZeilen = @(); $traeger = @()
    $ausRot = Join-Path $tmp 'bein-rot.txt'
    if (Test-Path $ausRot) { [IO.File]::Delete($ausRot) }
    try
    {
        foreach ($d in $dateien)
        {
            if (-not $mutiert) { continue }
            if ($texte[$d] -ne $utf8.GetString($orig[$d])) { Schreibe (Pfad $d) $utf8.GetBytes($texte[$d]) }
            $shaMut[$d] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Pfad $d)).Hash
        }
        $rot = Fahre $m.Test $ausRot
        if (Test-Path $ausRot)
        {
            $rotZeilen = @(Get-Content $ausRot -Encoding utf8)
            $traeger = @($rotZeilen | Where-Object { $_ -match $m.Traeger })
        }
    }
    finally
    {
        if ($mutiert) { foreach ($d in $dateien) { Schreibe (Pfad $d) $orig[$d] } }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Pfad $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $gruen = $null; $summeGruen = ''
    if ($mutiert)
    {
        $ausGruen = Join-Path $tmp 'bein-gruen.txt'
        if (Test-Path $ausGruen) { [IO.File]::Delete($ausGruen) }
        $gruen = Fahre $m.Test $ausGruen
        $gruenZeilen = if (Test-Path $ausGruen) { @(Get-Content $ausGruen -Encoding utf8) } else { @() }
        $summeGruen = [string](@($gruenZeilen | Where-Object { $_ -match '^SELBSTTEST' }) | Select-Object -Last 1)
    }

    $gruende = @()
    if ($null -eq $rot -or $rot.Exit -eq 0) { $gruende += "Lauf rot Exit $(if ($rot) { $rot.Exit } else { 'fehlt' })" }
    if ($m.Erwartung -eq 'rot' -and $traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if ($mutiert -and $gruen.Exit -ne 0) { $gruende += "Lauf gruen Exit $($gruen.Exit)" }
    $gueltigWort = if ($Art -eq 'vorher') { 'ROTLAUF GUELTIG' } else { 'ROTBEWEIS GUELTIG' }
    $ergebnis = if ($gruende.Count -eq 0) { $gueltigWort } else { $gueltigWort -replace 'GUELTIG', 'UNGUELTIG' }
    if ($gruende.Count -gt 0) { $ergebnis = "$ergebnis (" + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $ROHDATEI)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Nacharbeit 1 der Abschlusspruefung, $artWort $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Art: $artWort (§5.1, P-21 (g), §45): $($m.Beschreibung)")
    $zeilen.Add("Nakama HEAD $nakamaHead (Arbeitsstand der Nacharbeit 1 uncommittet); MCP-Repo HEAD $mcpHead (unveraendert)")
    $zeilen.Add("Bein einzeln: $($rot.Befehl) (Obergrenze $ObergrenzeSekunden s)")
    $zeilen.Add("Traeger (Muster des Traegers): $($m.Traeger)")
    $zeilen.Add("Erwartung: $($m.Erwartung), mindestens $($m.Mindestens) Traegerzeile(n)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        if ($mutiert) { $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })") }
        else { $zeilen.Add("SHA-256 $d (Traeger unveraendert) $($shaVor[$d]), nach dem Lauf $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })") }
    }
    $zeilen.Add("rot: Exit $($rot.Exit), Dauer $($rot.Dauer) s, Obergrenze erreicht: $(if ($rot.Obergrenze) { 'ja' } else { 'nein' }), Traegerzeilen $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add('--- Rohausgabe, rot ---')
    foreach ($z in $rotZeilen) { $zeilen.Add($z) }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    if ($mutiert) { $zeilen.Add("gruen: Exit $($gruen.Exit), Dauer $($gruen.Dauer) s, $summeGruen") }
    $zeilen.Add("Ergebnis: $ergebnis")
    $zeilen.Add('')
    # Anhaengen, nie ueberschreiben: ein frueherer Lauf derselben Zeile bleibt als Verlauf stehen.
    [IO.File]::AppendAllText($roh, (($zeilen -join "`n") + "`n"), $utf8)

    $shaText = ($dateien | ForEach-Object { "$(Split-Path $_ -Leaf) $($shaVor[$_].Substring(0,12))=$($shaNach[$_].Substring(0,12))" }) -join ', '
    $gruenText = if ($mutiert) { "gruen Exit $($gruen.Exit) ($($gruen.Dauer) s)" } else { 'ohne Gruenlauf (Traeger unveraendert)' }
    $zeile = "$Art $name | rot Exit $($rot.Exit) ($($rot.Dauer) s), Traeger $($traeger.Count) | $gruenText | $shaText | $ergebnis"
    $zusammenfassung.Add($zeile)
    Write-Output $zeile
    if ($gruende.Count -gt 0) { break }
}

if ($NurPruefen) { Write-Output "PRUEFUNG OK $geprueft von $($auswahl.Count)"; exit 0 }
[IO.File]::WriteAllText((Join-Path $tmp "zusammenfassung-$Art.txt"), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE ($Art): $($zusammenfassung.Count) Eintraege, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
