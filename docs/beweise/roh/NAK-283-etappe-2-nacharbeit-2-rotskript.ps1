# NAK-283 Etappe 2, Nacharbeit 2 — Rotbeweise der vier geaenderten Faelle
# (Bauer, 12.09.2026)
#
# Ablauf je Zeile nach docs/beweise/NAK-283.md §6.1:
#   1. SHA-256 der unveraenderten Quelldatei notieren
#   2. Mutation GENAU an der Zeile, die die Zusage traegt
#   3. LastWriteTime setzen (NAK-230: eine zurueckgespielte Quelle kann aelter
#      sein als ihr Objektfile; MSBuild baut dann nicht neu und meldet Exit 0)
#   4. Bein einzeln fahren -> erwartet ROT
#   5. Bytegleiche Ruecknahme (Originalbytes zurueckspielen), SHA-256 gegen
#      Schritt 1, Zeitstempel erneut setzen, Bein erneut fahren -> erwartet GRUEN
#
# GEGENUEBER DER NACHARBEIT 1 gefahren werden nur die Faelle aus B14, deren
# AUFBAU diese Runde geaendert hat (M-01, M-02, M-06, M-72); B27 und
# `Sonde014IntentTest.cpp` sind unberuehrt, ihre Rotbeweise bleiben gueltig
# (Manifest §26).
#
# ZWEI NEUE RIEGEL dieser Runde - sie pruefen genau das, was die Wiederpruefung 1
# beanstandet hat:
#   (a) ERWARTETE ZUSAGEZEILE: in JEDEM Rotlauf muss die Zeile fallen, die die
#       Zusage der Matrixzeile traegt. Ein Rotlauf, der nur an einem
#       Nebeneffekt faellt, gilt nicht als Beweis.
#   (b) KEINE AUFBAUZEILE DARF FALLEN: keine FEHLER-Zeile eines Rotlaufs darf
#       "Aufbau:" enthalten. Genau dort stand der Defekt dieser Runde - ein im
#       Fenster zwischen `bindungMutex` und Modellsperre einmalig ausgewerteter
#       Modellvergleich haette den Rotbeweis an einer Zeile mitfallen lassen,
#       die die Zusage nicht traegt (§6.1 Punkt 2).
# Dazu kommt der Erwartungshash je Produktdatei (§23.4): weicht er ab, ist der
# Produktcode angefasst worden und das Skript bricht ab, bevor es misst.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-rotskript.ps1 -Nur MUT-B
#
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-2-nacharbeit-2.txt

[CmdletBinding()]
param(
    [string] $Nur = '',
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
)

$ErrorActionPreference = 'Stop'

$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    foreach ($k in @(
        'C:\Program Files\CMake\bin\cmake.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe')) {
        if (Test-Path $k) { $cmake = $k; break }
    }
}
if (-not $cmake) { throw 'cmake nicht gefunden' }

$build     = Join-Path $Wurzel 'eq-copilot\build'
$rohOrdner = Join-Path $Wurzel 'docs\beweise\roh'

function Exe-Pfad([string] $bein) {
    Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $bein)
}

function Zeilenende([string] $text) {
    if ($text -match "`r`n") { return "`r`n" } else { return "`n" }
}

# Eine Mutation: Datei, Suchzeilen, Ersatzzeilen. Suchen und Ersetzen laufen
# zeilenweise und setzen das in der DATEI gefundene Zeilenende wieder ein -
# der Baum bleibt in seinen eigenen Zeilenenden (CLAUDE.md gitattributes).
function Mutiere([string] $datei, [string[]] $alt, [string[]] $neu) {
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    $suche = ($alt -join $le)
    if (-not $t.Contains($suche)) { throw "Mutationsmuster nicht gefunden in $datei" }
    $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
    if ($treffer -ne 1) { throw "Mutationsmuster $treffer-mal gefunden in $datei (erwartet: 1)" }
    $t = $t.Replace($suche, ($neu -join $le))
    [System.IO.File]::WriteAllText($datei, $t, (New-Object System.Text.UTF8Encoding $false))
}

function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }

function Baue([string[]] $beine) {
    $argumente = @('--build', $build, '--config', 'Release', '--target') + $beine
    $aus = & $cmake @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Fahre([string] $bein, [string[]] $argumente) {
    $exe = Exe-Pfad $bein
    $aus = & $exe @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function FehlerZeilen([string] $text) {
    return @($text -split "`n" | Where-Object { $_ -match 'FEHLER' } | ForEach-Object { $_.Trim() })
}

# -- Die Mutationen -------------------------------------------------------
$sourcesModel = Join-Path $Wurzel 'eq-copilot\plugin\src\SourcesModel.cpp'
$ipc          = Join-Path $Wurzel 'eq-copilot\plugin\src\prozessor\Ipc.cpp'

# Erwartungshashes aus Manifest §23.4 (dort bytegleich mit §20.2): der
# unabhaengige Beleg, dass diese Runde keinen Produktcode angefasst hat.
$erwarteterHash = @{
    $sourcesModel = '520A727382AAF6C0A8E1AA65D9BA973CC9E87B75825C8D2EB16CEDCB412373B3'
    $ipc          = 'B74AC525AD97CEEC36A0E0F56AA24599ECF7782B617164845A2B562C947B4B97'
}

$mutationen = @(
    [pscustomobject]@{
        Kennung = 'MUT-A'
        Zeilen  = 'M-01a, M-02, M-06'
        Datei   = $sourcesModel
        Was     = 'SourcesModel: die Folgenummernpruefung entfernen - nur der Generationsvergleich bleibt, wie am Basis-SHA'
        Alt     = @(
            '    if (folge <= zuletztUebernommeneFolge)',
            '    {',
            '        ++ueberholtZaehler;',
            '        return Publikation::ueberholt;',
            '    }')
        Neu     = @('    // MUTATION NAK-283 Rotbeweis: Folgenummernpruefung entfernt.')
        Beine   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Bein = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m01'); Datei = 'NAK-283-rot-M-01a-etappe-2-nacharbeit-2.txt'; Rotlaeufe = 1
               Erwartet = @('M-01: aeltere_mitgliederpublikation_ersetzt_keine_juengere') },
            @{ Bein = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m02'); Datei = 'NAK-283-rot-M-02-etappe-2-nacharbeit-2.txt';  Rotlaeufe = 1
               Erwartet = @('M-02: hauptziel_benennung_wird_nicht_von_aelterer_workerkopie_ueberholt') },
            @{ Bein = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m06'); Datei = 'NAK-283-rot-M-06-etappe-2-nacharbeit-2.txt';  Rotlaeufe = 1
               Erwartet = @('M-06: state_und_modell_sind_nach_ruhe_gleich') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-B'
        Zeilen  = 'M-01b, M-72 (je drei Rotlaeufe)'
        Datei   = $ipc
        Was     = 'Ipc: den Ueberholt-Ausstieg auf den Reload-Zweig legen (Ausstieg VOR meldeHostDirty und v3StateRevision)'
        Alt     = @(
            '            sourcesPublikationUeberholt.fetch_add (1);',
            '            break;')
        Neu     = @(
            '            sourcesPublikationUeberholt.fetch_add (1);',
            '            return;   // MUTATION NAK-283 Rotbeweis: Ausstieg vor Dirty und Revision')
        Beine   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Bein = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m01'); Datei = 'NAK-283-rot-M-01b-etappe-2-nacharbeit-2.txt'; Rotlaeufe = 3
               Erwartet = @('M-01: Dirty-Zaehler == Zahl der State-aendernden Befehle', 'M-01: Revision-Delta == 2') },
            @{ Bein = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m72'); Datei = 'NAK-283-rot-M-72-etappe-2-nacharbeit-2.txt';  Rotlaeufe = 3
               Erwartet = @('M-72 Phase B: reloadablehnung_und_ueberholung_sind_unterscheidbar') })
    }
)

$bilanz = @()
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ($($m.Zeilen)) ===" -ForegroundColor Cyan

    $sicherung  = [System.IO.File]::ReadAllBytes($m.Datei)
    $hashVorher = Hash-Von $m.Datei
    $soll = $erwarteterHash[$m.Datei]
    if ($soll -and $hashVorher -ne $soll) {
        throw "Quell-Hash von $($m.Datei) weicht von §23.4 ab ($hashVorher statt $soll) - Produktcode angefasst?"
    }

    $kopfZeilen = @(
        "NAK-283 Etappe 2, Nacharbeit 2 - Rotbeweis $($m.Kennung) fuer $($m.Zeilen)",
        "Datei:      $($m.Datei.Replace($Wurzel + '\', ''))",
        "Mutation:   $($m.Was)",
        "SHA-256 vorher: $hashVorher  (= Manifest §23.4: $(if ($soll -and $hashVorher -eq $soll) { 'ja' } else { 'ohne Erwartung' }))",
        "")

    try {
        Mutiere $m.Datei $m.Alt $m.Neu
        (Get-Item $m.Datei).LastWriteTime = Get-Date
        $bau = Baue $m.Beine
        if ($bau.ExitCode -ne 0) { throw "Bau der mutierten Fassung fehlgeschlagen: $($bau.Text)" }
        foreach ($b in $m.Beine) {
            $exe = Exe-Pfad $b
            if ((Get-Item $exe).LastWriteTime -lt (Get-Item $m.Datei).LastWriteTime) {
                throw "Binary $b ist aelter als die mutierte Quelle - MSBuild hat nicht neu gebaut"
            }
        }
        $rotLaeufe = @()
        foreach ($l in $m.Laeufe) {
            $wdh = if ($l.ContainsKey('Rotlaeufe')) { [int] $l.Rotlaeufe } else { 1 }
            $ergebnisse = @()
            for ($i = 1; $i -le $wdh; $i++) {
                $r = Fahre $l.Bein $l.Arg
                $ergebnisse += $r
                Write-Host ("  ROT   {0} {1} (Lauf {2}/{3}) -> Exit {4}" -f $l.Bein, ($l.Arg -join ' '), $i, $wdh, $r.ExitCode)
            }
            $rotLaeufe += [pscustomobject]@{ Lauf = $l; Ergebnisse = $ergebnisse }
        }
    }
    finally {
        # Bytegleiche Ruecknahme: die ORIGINALBYTES zurueck, nie eine zweite
        # Textersetzung - so ist die Gleichheit strukturell und der Hash belegt sie.
        [System.IO.File]::WriteAllBytes($m.Datei, $sicherung)
    }

    $hashNachher = Hash-Von $m.Datei
    if ($hashNachher -ne $hashVorher) { throw "Ruecknahme NICHT bytegleich in $($m.Datei)" }
    (Get-Item $m.Datei).LastWriteTime = Get-Date
    $bau2 = Baue $m.Beine
    if ($bau2.ExitCode -ne 0) { throw "Bau der zurueckgespielten Fassung fehlgeschlagen: $($bau2.Text)" }

    foreach ($e in $rotLaeufe) {
        $l = $e.Lauf
        $gruen = Fahre $l.Bein $l.Arg
        Write-Host ("  GRUEN {0} {1} -> Exit {2}" -f $l.Bein, ($l.Arg -join ' '), $gruen.ExitCode)
        $anzahl  = $e.Ergebnisse.Count
        $rotExits = @($e.Ergebnisse | ForEach-Object { $_.ExitCode })

        # Riegel (a): die Zusagezeile faellt in JEDEM Rotlauf.
        # Riegel (b): keine Aufbauzeile faellt in IRGENDEINEM Rotlauf.
        $fehlendeZusage = @()
        $gefalleneAufbauzeilen = @()
        for ($i = 0; $i -lt $anzahl; $i++) {
            $fz = FehlerZeilen $e.Ergebnisse[$i].Text
            foreach ($muster in $l.Erwartet) {
                if (-not ($fz | Where-Object { $_ -like "*$muster*" })) {
                    $fehlendeZusage += ("Lauf {0}: '{1}' faellt NICHT" -f ($i + 1), $muster)
                }
            }
            $gefalleneAufbauzeilen += @($fz | Where-Object { $_ -match 'Aufbau:' } | ForEach-Object { "Lauf $($i + 1): $_" })
        }
        # Der Gruenlauf darf ueberhaupt keine FEHLER-Zeile tragen.
        $gruenFehler = FehlerZeilen $gruen.Text

        $ausgabe = Join-Path $rohOrdner $l.Datei
        # Jedes Muster als EIGENE Zeile: ein verschachteltes Array wuerde von
        # `WriteAllLines` ueber `$OFS` zu einer einzigen Zeile verbunden.
        $musterZeilen = @($l.Erwartet | ForEach-Object { "  $_" })
        $inhalt = $kopfZeilen + @(
            "Bein:       $($l.Bein) $($l.Arg -join ' ')",
            "Rotlaeufe:  $anzahl (Exit je Lauf: $($rotExits -join ', '))",
            "SHA-256 nachher (bytegleiche Ruecknahme): $hashNachher",
            "Urteil:     ROT Exit $($rotExits -join '/') / GRUEN Exit $($gruen.ExitCode)",
            "",
            "--- RIEGEL DIESER RUNDE ---",
            "Zusagezeile(n), die in JEDEM Rotlauf fallen muessen:") + $musterZeilen + @(
            ("  Ergebnis: {0}" -f $(if ($fehlendeZusage.Count -eq 0) { "erfuellt in allen $anzahl Rotlaeufen" } else { "NICHT ERFUELLT - " + ($fehlendeZusage -join '; ') })),
            "Keine FEHLER-Zeile eines Rotlaufs darf 'Aufbau:' enthalten (§6.1 Punkt 2):",
            ("  Ergebnis: {0}" -f $(if ($gefalleneAufbauzeilen.Count -eq 0) { 'erfuellt - keine Aufbauzeile ist gefallen' } else { 'NICHT ERFUELLT - ' + ($gefalleneAufbauzeilen -join ' | ') })),
            ("Gruenlauf ohne FEHLER-Zeile: {0}" -f $(if ($gruenFehler.Count -eq 0) { 'ja' } else { 'NEIN - ' + ($gruenFehler -join ' | ') })),
            "")
        for ($i = 0; $i -lt $anzahl; $i++) {
            $inhalt += @(
                ("--- MUTIERT, Lauf {0} von {1} (erwartet: rot) ---------------------" -f ($i + 1), $anzahl),
                $e.Ergebnisse[$i].Text,
                "")
        }
        $inhalt += @(
            "--- ZURUECKGESPIELT (erwartet: gruen) -----------------------------",
            $gruen.Text)
        [System.IO.File]::WriteAllLines($ausgabe, $inhalt, (New-Object System.Text.UTF8Encoding $false))

        $schlechtesterRot = ($rotExits | Measure-Object -Minimum).Minimum
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Bein = "$($l.Bein) $($l.Arg -join ' ')"
            Rotlaeufe = $anzahl; RotMin = $schlechtesterRot; Gruen = $gruen.ExitCode
            HashGleich = ($hashNachher -eq $hashVorher)
            Zusage = ($fehlendeZusage.Count -eq 0); OhneAufbau = ($gefalleneAufbauzeilen.Count -eq 0)
            Datei = $l.Datei
        }
    }
}

Write-Host ''
Write-Host '=== Bilanz ===' -ForegroundColor Cyan
$bilanz | Format-Table -AutoSize
$schlecht = @($bilanz | Where-Object {
    $_.RotMin -eq 0 -or $_.Gruen -ne 0 -or -not $_.HashGleich -or -not $_.Zusage -or -not $_.OhneAufbau })
if ($schlecht.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($schlecht.Count) Rotbeweis(e)" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Rotbeweise bestanden (jeder Rotlauf != 0 an der Zusagezeile, keine Aufbauzeile gefallen, gruen == 0, Hash bytegleich)" -ForegroundColor Green
exit 0
