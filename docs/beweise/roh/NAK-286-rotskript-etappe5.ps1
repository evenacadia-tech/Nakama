<#
NAK-286 Etappe 5 - Rotlaeufe vor der Aenderung und Rotbeweise nach dem Bau (Manifest docs/beweise/NAK-286.md §43;
§5.1, Regeln P-17, P-18 und P-20; Zeilen M-21, M-64, M-74). Muster: NAK-286-rotskript-etappe4-nacharbeit1.ps1.

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe5.ps1 -Art vorher   Rotlaeufe vor der Aenderung
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe5.ps1 -Art nach     Rotbeweise nach dem Bau
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe5.ps1 -PruefeM74    nur die Markerpruefung M-74
  ... -Nur M-21,M-64   nur diese Zeilen        ... -NurPruefen   nur Traegertexte zaehlen

Je Eintrag, in dieser Reihenfolge (§5.1; PowerShell, Python, JSON und Markdown ohne den LastWriteTime-Schritt):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot5).
  2. Mutation genau an der Traegerzeile, jeder Alttext genau einmal (P-20: haelt eine redundante Schranke die
     Zusage, gehoert sie zur Mutation). Ein Rotlauf vor der Aenderung faehrt den neuen Fall gegen den noch
     unveraenderten Traeger und mutiert nichts.
  3. Das Bein einzeln, mit Obergrenze: A35 als einzelner Selbsttestfall (laufzeit.ps1 -Selbsttest -Nur,
     szenario.py --selbsttest --nur) oder die Markerpruefung M-74 (git grep an den Stellen aus §0.6, dieses Skript
     mit -PruefeM74). Nach Ablauf der Obergrenze wird der Prozessbaum beendet (taskkill /T /F); das Ende an der
     Obergrenze steht in der Rohdatei. Erwartung "rot": Exit ungleich 0 und mindestens die verlangte Zahl Zeilen,
     die den Traeger nennen (Muster des Traegers, nie der Meldung).
  4. Bytegleiche Ruecknahme aus den gesicherten Bytes (Schreiben mit Wiederholung), SHA-256 gleich Schritt 1,
     derselbe Lauf gruen.
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-21-etappe5.txt, NAK-286-rot-M-64-etappe5.txt, NAK-286-rot-M-74.txt; je
Eintrag ein angehaengter Abschnitt. Der Lauf startet kein FL, installiert nichts und rendert nichts: jedes Bein laeuft
gegen Attrappen oder liest nur Git.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen, [ValidateSet('vorher', 'nach')] [string] $Art = 'nach',
      [int] $ObergrenzeSekunden = 240, [switch] $PruefeM74)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$utf8 = New-Object System.Text.UTF8Encoding($false)

# ================================================================ Markerpruefung M-74 (Bein der Zeile)
if ($PruefeM74)
{
    # M-74 (§4.11): "anzulegen" fuer S25e steht an keiner Stelle aus §0.6 mehr, tools/fl/LIES-MICH.md nennt keine
    # Grenze der ersten Fassung, und .claude/skills/dirigent/SKILL.md:366 bleibt (der verbleibende Handgriff, kein
    # Marker). Andere Registerzeilen mit dem Wort gehoeren nicht zu S25e und stehen nur als Kontext da.
    [Console]::OutputEncoding = $utf8
    $stellen = @(
        [pscustomobject]@{ Datei = 'docs/gesundheit/KONZEPT.md'; Muster = 'anzulegen'; Zeile = '.'; Stelle = 'KONZEPT §4.6, Szenarien je Karte' }
        [pscustomobject]@{ Datei = 'docs/offene-punkte.md'; Muster = 'anzulegen'; Zeile = '^\| NAK-284 \|'; Stelle = 'Register NAK-284' }
        [pscustomobject]@{ Datei = 'docs/offene-punkte.md'; Muster = 'anzulegen'; Zeile = '^\| NAK-286 \|'; Stelle = 'Register NAK-286' }
        [pscustomobject]@{ Datei = 'docs/gesundheit/szenarien/LIES-MICH.md'; Muster = 'anzulegen'; Zeile = '.'; Stelle = 'Szenarien, Absatz Noch anzulegen (S25e)' }
        [pscustomobject]@{ Datei = 'tools/fl/LIES-MICH.md'; Muster = 'Grenzen der ersten Fassung'; Zeile = '.'; Stelle = 'Grenzen der ersten Fassung' })
    $treffer = 0
    foreach ($gruppe in @($stellen | Group-Object Datei, Muster))
    {
        $datei = $gruppe.Group[0].Datei
        $muster = $gruppe.Group[0].Muster
        Write-Output "git grep -n -F -e '$muster' -- $datei"
        foreach ($z in @(& git -C $wurzel grep -n -F -e $muster -- $datei))
        {
            if ($z -notmatch '^([^:]+):(\d+):(.*)$') { Write-Output "  unlesbar: $z"; continue }
            $pfad = $Matches[1]; $nr = $Matches[2]; $inhalt = $Matches[3]
            $i = [math]::Max(0, $inhalt.IndexOf($muster, [StringComparison]::Ordinal))
            $von = [math]::Max(0, $i - 90)
            $bis = [math]::Min($inhalt.Length, $i + $muster.Length + 40)
            $auszug = $inhalt.Substring($von, $bis - $von)
            $s = @($gruppe.Group | Where-Object { $inhalt -match $_.Zeile })
            if ($s.Count -gt 0) { $treffer++; Write-Output "TREFFER ${pfad}:$nr ($($s[0].Stelle)): ...$auszug..." }
            else { Write-Output "  nicht S25e (andere Registerzeile, bleibt): ${pfad}:$nr ...$auszug..." }
        }
    }
    Write-Output "git grep -n -F -e 'Diagnoseprojekt nach NAK-30 neu anzulegen' -- .claude/skills/dirigent/SKILL.md"
    $skill = @(& git -C $wurzel grep -n -F -e 'Diagnoseprojekt nach NAK-30 neu anzulegen' -- .claude/skills/dirigent/SKILL.md)
    foreach ($z in $skill) { Write-Output "BLEIBT $z" }
    if ($skill.Count -eq 0) { Write-Output 'FEHLT: .claude/skills/dirigent/SKILL.md traegt den verbleibenden Handgriff nicht mehr' }
    if ($treffer -gt 0 -or $skill.Count -eq 0)
    {
        Write-Output "M-74 ROT: $treffer Treffer an Stellen aus §0.6$(if ($skill.Count -eq 0) { '; Skill-Stelle fehlt' })"
        exit 1
    }
    Write-Output 'M-74 GRUEN: keine Stelle aus §0.6 traegt den Marker; die Skill-Stelle bleibt'
    exit 0
}

$tmp = Join-Path $env:TEMP 'nak286-rot5'
New-Item -ItemType Directory -Force $tmp | Out-Null
$env:PYTHONIOENCODING = 'utf-8'
$env:PYTHONDONTWRITEBYTECODE = '1'

$LZ = 'tools/fl/laufzeit.ps1'
$SZ = 'tools/fl/szenario.py'
$NH = 'docs/gesundheit/szenarien/nulltest-host.json'
$KZ = 'docs/gesundheit/KONZEPT.md'
$OP = 'docs/offene-punkte.md'
$SL = 'docs/gesundheit/szenarien/LIES-MICH.md'
$FL = 'tools/fl/LIES-MICH.md'
$MCP = 'C:\Users\phili\Projekte\fl-studio-mcp'

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
                  [string] $roh, [string] $erwartung, [string[]] $quellen, [object[]] $ersetzungen, [int] $mindestens = 1)
{
    [pscustomobject]@{ Art = $art; Zeile = $zeile; Teil = $teil; Beschreibung = $beschreibung; Test = $test; Traeger = $traeger
                       Roh = $roh; Erwartung = $erwartung; Quellen = $quellen; Ersetzungen = $ersetzungen; Mindestens = $mindestens }
}

$eintraege = New-Object System.Collections.Generic.List[object]

# ================================================================ Rotlaeufe vor der Aenderung
# P-17 (M-21): der neue Fall mit dem Bild aus L-286-1 gegen das unveraenderte lokal.fenster (ohne Groessenpruefung).
$eintraege.Add((Eintrag 'vorher' 'M-21' '(P-17)' 'neuer Fall fenster_eingeklappt gegen das unveraenderte lokal.fenster (ohne Groessenpruefung)' 'sz:fenster_eingeklappt' '\[ROT\] M-21 fenster_eingeklappt:.*Plugin-Fensterbild 67 x 31 nicht als eingeklappt: Exit 0 statt 5' 'NAK-286-rot-M-21-etappe5.txt' 'rot' @($SZ) @()))

# P-18 (M-64): der neue Fall mit der Szenariodatei des Repos gegen den unveraenderten Runner (Kopie und SHA-256 nur des
# Diagnoseprojekts) und die unveraenderte nulltest-host.json (fruehere Namen).
$eintraege.Add((Eintrag 'vorher' 'M-64' '(P-18)' 'neuer Fall referenzprojekte_aus_szenariodatei gegen den unveraenderten Runner und die unveraenderte nulltest-host.json' 'ps:referenzprojekte_aus_szenariodatei' '\[ROT\] M-64 referenzprojekte_aus_szenariodatei:.*Arbeitskopie traegt \[Nakama-Diagnose\.flp\] statt \[' 'NAK-286-rot-M-64-etappe5.txt' 'rot' @($LZ, $NH) @()))

# M-74: die Markerpruefung am Basisstand - die vier Stellen aus §0.6 mit dem Wort treffen.
$eintraege.Add((Eintrag 'vorher' 'M-74' '' 'Markerpruefung (git grep an den Stellen aus §0.6) am Basisstand' 'gg:m74' '^TREFFER ' 'NAK-286-rot-M-74.txt' 'rot' @($KZ, $OP, $SL, $FL) @() 4))

# ================================================================ Rotbeweise nach dem Bau
# M-21 (P-17): an der Groessenpruefung in lokal.fenster - entfernt, beide Masse verlangt, auch fuer das Hauptfenster.
$eintraege.Add((Eintrag 'nach' 'M-21' '(P-17)' 'Groessenpruefung in lokal.fenster entfernt' 'sz:fenster_eingeklappt' '\[ROT\] M-21 fenster_eingeklappt:.*Plugin-Fensterbild 67 x 31 nicht als eingeklappt: Exit 0 statt 5' 'NAK-286-rot-M-21-etappe5.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    eingeklappt = (ziel == "plugin" and _ist_zahl(breite) and _ist_zahl(hoehe)
                   and (breite < PLUGIN_FENSTER_MIN[0] or hoehe < PLUGIN_FENSTER_MIN[1]))
'@ @'
    eingeklappt = False  # ROTBEWEIS M-21 (P-17): Groessenpruefung entfernt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-21' '(P-17 Masse)' 'Groessenpruefung verlangt beide Masse unter der Mindestgroesse statt eines' 'sz:fenster_eingeklappt' '\[ROT\] M-21 fenster_eingeklappt:.*Mindestgroesse 200 x 100: Plugin-Fensterbild 199 x 480 endet mit Exit 0 statt 5' 'NAK-286-rot-M-21-etappe5.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                   and (breite < PLUGIN_FENSTER_MIN[0] or hoehe < PLUGIN_FENSTER_MIN[1]))
'@ @'
                   and (breite < PLUGIN_FENSTER_MIN[0] and hoehe < PLUGIN_FENSTER_MIN[1]))  # ROTBEWEIS M-21 (P-17 Masse): beide Masse verlangt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-21' '(P-17 Hauptfenster)' 'Groessenpruefung auch fuer ziel fl (das FL-Hauptfenster)' 'sz:fenster_eingeklappt' '\[ROT\] M-21 fenster_eingeklappt:.*FL-Hauptfenster 67 x 31 als eingeklappter Wrapper gewertet: Exit 5' 'NAK-286-rot-M-21-etappe5.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    eingeklappt = (ziel == "plugin" and _ist_zahl(breite) and _ist_zahl(hoehe)
'@ @'
    eingeklappt = (_ist_zahl(breite) and _ist_zahl(hoehe)  # ROTBEWEIS M-21 (P-17 Hauptfenster): auch ziel fl
'@))))

# M-64 (P-18): am Namen in nulltest-host.json (je Schritt), am Vergleich am Ende und an der Auswahl nach Namen.
$eintraege.Add((Eintrag 'nach' 'M-64' '(P-18 Name Referenz)' 'Schritt ohne_slots nennt statt Nakama-Diagnose-Referenz.flp den frueheren Namen Nakama-Diagnose-ohne-Slots.flp (Koeder im Selbsttest)' 'ps:referenzprojekte_aus_szenariodatei' '\[ROT\] M-64 referenzprojekte_aus_szenariodatei:.*Referenzprojekt Nakama-Diagnose-Referenz\.flp ohne SHA-256 von Repo und Arbeitskopie am Anfang' 'NAK-286-rot-M-64-etappe5.txt' 'rot' @() @(
    (Ersetzung $NH @'
"projekt": "Nakama-Diagnose-Referenz.flp"
'@ @'
"projekt": "Nakama-Diagnose-ohne-Slots.flp"
'@))))

$eintraege.Add((Eintrag 'nach' 'M-64' '(P-18 Name Verarbeitung)' 'Schritt verarbeitung_ein nennt statt Nakama-Diagnose-Verarbeitung.flp den frueheren Namen Nakama-Diagnose-Verarbeitung-ein.flp (Koeder im Selbsttest)' 'ps:referenzprojekte_aus_szenariodatei' '\[ROT\] M-64 referenzprojekte_aus_szenariodatei:.*Referenzprojekt Nakama-Diagnose-Verarbeitung\.flp ohne SHA-256 von Repo und Arbeitskopie am Anfang' 'NAK-286-rot-M-64-etappe5.txt' 'rot' @() @(
    (Ersetzung $NH @'
"projekt": "Nakama-Diagnose-Verarbeitung.flp"
'@ @'
"projekt": "Nakama-Diagnose-Verarbeitung-ein.flp"
'@))))

$eintraege.Add((Eintrag 'nach' 'M-64' '(P-18 Hash am Ende)' 'Vergleich der Referenzprojekte am Ende (Pruefe-Projekt) entfernt' 'ps:referenzprojekte_aus_szenariodatei' '\[ROT\] M-64 referenzprojekte_aus_szenariodatei:.*veraenderte Referenzprojekte \(Arbeitskopie, Repo\): Exit 0 GEMESSEN' 'NAK-286-rot-M-64-etappe5.txt' 'rot' @() @(
    (Ersetzung $LZ @'
    # P-18: M-12 gilt fuer alle drei Projekte - jedes kopierte Referenzprojekt wird wie das Diagnoseprojekt verglichen.
    foreach ($name in @($K.ReferenzHashes.Keys)) {
        $h = $K.ReferenzHashes[$name]
        $r = & $script:U.Hash $h.Quelle
        $c = & $script:U.Hash $h.Ziel
        Log ("Referenzprojekt {0} am Ende: SHA-256 Repo {1}, Arbeitskopie {2}" -f $name, $r, $c)
        if ($r -ne $h.Repo -or $c -ne $h.Kopie) { $gruende += "Referenzprojekt $name veraendert (Repo $($h.Repo) -> $r, Arbeitskopie $($h.Kopie) -> $c)" }
    }
'@ @'
    # ROTBEWEIS M-64 (P-18 Hash am Ende): Vergleich der Referenzprojekte am Ende entfernt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-64' '(P-18 Muster)' 'Namen aus der Szenariodatei durch ein Suchmuster im Ordner des Diagnoseprojekts ersetzt' 'ps:referenzprojekte_aus_szenariodatei' '\[ROT\] M-64 referenzprojekte_aus_szenariodatei:.*Arbeitskopie traegt \[[^\]]*Nakama-Diagnose-ohne-Slots\.flp' 'NAK-286-rot-M-64-etappe5.txt' 'rot' @() @(
    (Ersetzung $LZ @'
    foreach ($name in @(Referenzprojekt-Namen $liste)) {
'@ @'
    foreach ($name in @(& $script:U.Namen $ordner | Where-Object { $_ -like 'Nakama-Diagnose-*.flp' })) {  # ROTBEWEIS M-64 (P-18 Muster): Suchen nach einem Muster
'@))))

# M-74: je Stelle aus §0.6 die Zeile zurueckgespielt - die Markerpruefung trifft.
$eintraege.Add((Eintrag 'nach' 'M-74' '(KONZEPT)' 'Wort (anzulegen) in docs/gesundheit/KONZEPT.md zurueckgespielt' 'gg:m74' '^TREFFER docs/gesundheit/KONZEPT\.md:\d+' 'NAK-286-rot-M-74.txt' 'rot' @() @(
    (Ersetzung $KZ @'
`docs/gesundheit/szenarien/`, Telemetrie-Leser
'@ @'
`docs/gesundheit/szenarien/` (anzulegen), Telemetrie-Leser
'@))))

$eintraege.Add((Eintrag 'nach' 'M-74' '(NAK-284)' 'Wort (anzulegen) in der Registerzeile NAK-284 zurueckgespielt' 'gg:m74' '^TREFFER docs/offene-punkte\.md:\d+ \(Register NAK-284\)' 'NAK-286-rot-M-74.txt' 'rot' @() @(
    (Ersetzung $OP @'
Messszenarien je Karte unter `docs/gesundheit/szenarien/`, Telemetrie-Leser
'@ @'
Messszenarien je Karte unter `docs/gesundheit/szenarien/` (anzulegen), Telemetrie-Leser
'@))))

$eintraege.Add((Eintrag 'nach' 'M-74' '(NAK-286)' 'Wort (anzulegen) in der Registerzeile NAK-286 zurueckgespielt' 'gg:m74' '^TREFFER docs/offene-punkte\.md:\d+ \(Register NAK-286\)' 'NAK-286-rot-M-74.txt' 'rot' @() @(
    (Ersetzung $OP @'
Manifest `docs/beweise/NAK-286.md`: Bereitschaftslauf
'@ @'
Manifest `docs/beweise/NAK-286.md` (anzulegen): Bereitschaftslauf
'@))))

$eintraege.Add((Eintrag 'nach' 'M-74' '(Szenarien)' 'Absatz Noch anzulegen (S25e) in docs/gesundheit/szenarien/LIES-MICH.md zurueckgespielt' 'gg:m74' '^TREFFER docs/gesundheit/szenarien/LIES-MICH\.md:\d+' 'NAK-286-rot-M-74.txt' 'rot' @() @(
    (Ersetzung $SL @'
Gebaut in S25e (NAK-286, Etappen 2 bis 5): Nulltest im echten Host
'@ @'
Noch anzulegen (S25e): Nulltest im echten Host
'@))))

$eintraege.Add((Eintrag 'nach' 'M-74' '(Grenzen)' 'Grenze der ersten Fassung in tools/fl/LIES-MICH.md zurueckgespielt' 'gg:m74' '^TREFFER tools/fl/LIES-MICH\.md:\d+' 'NAK-286-rot-M-74.txt' 'rot' @() @(
    (Ersetzung $FL @'
Screenshot, Nulltest im Host (Auslieferungszustand) und Diagnose-Briefkasten
'@ @'
Grenzen der ersten Fassung: kein Screenshot, kein Nulltest im Host, kein Diagnose-Briefkasten. Screenshot, Nulltest im Host (Auslieferungszustand) und Diagnose-Briefkasten
'@))))

# ================================================================ Lauf
function Fahre ([string] $test, [string] $ausgabe)
{
    $art, $rest = $test -split ':', 2
    $befehl = switch ($art)
    {
        'ps' { @('pwsh', '-NoProfile', '-File', $LZ, '-Selbsttest', '-Nur', $rest) }
        'sz' { @('py', '-3.13', $SZ, '--selbsttest', '--nur', $rest) }
        'gg' { @('pwsh', '-NoProfile', '-File', $PSCommandPath, '-PruefeM74') }
        default { throw "unbekannte Testart $art" }
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

# Mit `pwsh -File` kommt `-Nur M-21,M-64` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($eintraege | Where-Object { $_.Art -eq $Art -and ($Nur.Count -eq 0 -or $Nur -contains $_.Zeile -or $Nur -contains ($_.Zeile + ' ' + $_.Teil).Trim()) })
$zusammenfassung = New-Object System.Collections.Generic.List[string]
$nakamaHead = (& git -C $wurzel rev-parse HEAD)
$mcpHead = (& git -C $MCP rev-parse HEAD)
$artWort = @{ vorher = 'Rotlauf vor der Aenderung'; nach = 'Rotbeweis' }[$Art]

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
    if ($NurPruefen) { Write-Output "PRUEFUNG OK $Art $name"; continue }

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
        $summeGruen = [string](@($gruenZeilen | Where-Object { $_ -match '^(SELBSTTEST|M-74 )' }) | Select-Object -Last 1)
    }

    $gruende = @()
    if ($null -eq $rot -or $rot.Exit -eq 0) { $gruende += "Lauf rot Exit $(if ($rot) { $rot.Exit } else { 'fehlt' })" }
    if ($m.Erwartung -eq 'rot' -and $traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if ($mutiert -and $gruen.Exit -ne 0) { $gruende += "Lauf gruen Exit $($gruen.Exit)" }
    $gueltigWort = if ($Art -eq 'vorher') { 'ROTLAUF GUELTIG' } else { 'ROTBEWEIS GUELTIG' }
    $ergebnis = if ($gruende.Count -eq 0) { $gueltigWort } else { $gueltigWort -replace 'GUELTIG', 'UNGUELTIG' }
    if ($gruende.Count -gt 0) { $ergebnis = "$ergebnis (" + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $m.Roh)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 5, $artWort $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Art: $artWort (§5.1, §43): $($m.Beschreibung)")
    $zeilen.Add("Nakama HEAD $nakamaHead (Arbeitsstand der Etappe 5 uncommittet); MCP-Repo HEAD $mcpHead (unveraendert)")
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

[IO.File]::WriteAllText((Join-Path $tmp "zusammenfassung-$Art.txt"), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE ($Art): $($zusammenfassung.Count) Eintraege, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
