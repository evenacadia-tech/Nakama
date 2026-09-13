# NAK-283 Etappe 6 — Rotbeweise je Matrixzeile M-61 bis M-70 (Bauer, 13.09.2026)
#
# Ablauf je Mutation nach docs/beweise/NAK-283.md §6.1:
#   1. Originalbytes und SHA-256 JEDER Datei der Mutation sichern - auch der
#      Dateien, die ein Vorlauf ueber ihren Erzeuger neu schreibt
#   2. Mutation genau an der Zeile, die die Zusage traegt - jedes Muster genau
#      einmal (sonst Abbruch), die Zeilenenden der Datei bleiben; danach der
#      Vorlauf, falls einer steht
#   3. LastWriteTime setzen (NAK-230); Neubau der betroffenen Ziele, jedes
#      Binary muss juenger sein als jede mutierte Datei
#   4. Lauf -> ROT zaehlt nur mit dem erwarteten roten Exit UND einer
#      Traegerzeile (rote Marke plus Traeger der Zusage), nie an einem
#      Nebeneffekt
#   5. Originalbytes zurueck (nie eine zweite Textersetzung), SHA-256 gegen
#      Schritt 1, Zeitstempel, Neubau, Lauf -> GRUEN mit dem erwarteten gruenen
#      Exit UND gruener Traegerzeile
#
# Basislaeufe (ROT am Basis-SHA, keine Mutation): derselbe Riegel mit
# `--revision <SHA>` rot und am Arbeitsbaum gruen.
# Beobachtungslaeufe zeigen, was ein Bein unter der Mutation sieht; sie zaehlen
# nicht als Rotbeweis. Die Matrixmutation von M-65 (8-fach auf 4-fach) laeuft
# woertlich und wird als BEOBACHTET gefuehrt - ihr Ergebnis ist eine
# Abweichung (Manifest §36.6); den Rotbeweis an der Zusagezeile traegt MUT-65b.
# Keine Zeile dieser Etappe ist Nebenlaeufigkeitszeile.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-6-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-6-rotskript.ps1 -Nur MUT-61
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-6.txt,
# Bilanz:      docs/beweise/roh/NAK-283-etappe-6-rotbilanz.txt (nur beim vollen Lauf)

[CmdletBinding()]
param(
    [string] $Nur = '',
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
)

$ErrorActionPreference = 'Stop'
Set-Location $Wurzel
# Die Pruefausgaben tragen ±, − und ·: Python schreibt UTF-8, pwsh liest UTF-8.
$env:PYTHONUTF8 = '1'
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)

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
$utf8      = New-Object System.Text.UTF8Encoding $false
$basis     = '7b1f38b7bddcf67965d1bbc2609fd74464cfaaea'   # Basis-SHA der Etappe 6
$etappe1   = '3c5ec9ddd880c4d559e6ed343246da21b86adba6'   # Basis-SHA der Etappe 1 (Matrix geschrieben)
$riegel    = 'docs\beweise\roh\NAK-283-etappe-6-riegel.py'
$soak      = 'tools\eq-copilot\pruefe_session_soak.py'

function Exe-Pfad([string] $ziel) { Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel) }
function Zeilenende([string] $text) { if ($text -match "`r`n") { "`r`n" } else { "`n" } }
function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }
function Relativ([string] $pfad) { $pfad.Replace($Wurzel + '\', '').Replace('\', '/') }

function Mutiere([string] $datei, [string[]] $alt, [string[]] $neu) {
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    $suche = ($alt -join $le)
    $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
    if ($treffer -ne 1) { throw "Mutationsmuster $treffer-mal gefunden in $(Relativ $datei) (erwartet: 1): $($alt[0])" }
    $t = $t.Replace($suche, ($neu -join $le))
    [System.IO.File]::WriteAllText($datei, $t, $utf8)
}

function Baue([string[]] $ziele) {
    if ($ziele.Count -eq 0) { return @{ ExitCode = 0; Text = '(kein Bauziel)' } }
    $argumente = @('--build', $build, '--config', 'Release', '--target') + $ziele
    $aus = & $cmake @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = (($aus | ForEach-Object { "$_" }) -join "`n") }
}

function Pruefe-Frische([string[]] $ziele, [string[]] $dateien, [string] $phase) {
    $zeilen = @()
    foreach ($z in $ziele) {
        $exe = Get-Item (Exe-Pfad $z)
        foreach ($d in $dateien) {
            if ($exe.LastWriteTime -lt (Get-Item $d).LastWriteTime) {
                throw "Binary $z ist aelter als $(Relativ $d) ($phase) - MSBuild hat nicht neu gebaut"
            }
        }
        $zeilen += "$z $($exe.LastWriteTime.ToString('HH:mm:ss.fff'))"
    }
    return ($zeilen -join ', ')
}

function Fahre($lauf, [string] $phase) {
    $argumente = $lauf.Arg
    if ($phase -eq 'rot'   -and $lauf.ContainsKey('ArgRot'))   { $argumente = $lauf.ArgRot }
    if ($phase -eq 'gruen' -and $lauf.ContainsKey('ArgGruen')) { $argumente = $lauf.ArgGruen }
    if ($null -eq $argumente) { $argumente = @() }
    $t0 = Get-Date
    if ($lauf.Art -eq 'py') {
        $aus = & py -3.13 (Join-Path $Wurzel $lauf.Skript) @argumente 2>&1
        $befehl = "py -3.13 $($lauf.Skript.Replace('\', '/')) $($argumente -join ' ')".TrimEnd()
    } else {
        $aus = & (Exe-Pfad $lauf.Ziel) @argumente 2>&1
        $befehl = "$($lauf.Ziel) $($argumente -join ' ')".TrimEnd()
    }
    $code = $LASTEXITCODE
    return @{ ExitCode = $code; Text = (($aus | ForEach-Object { "$_" }) -join "`n"); Befehl = $befehl
              Sekunden = [math]::Round(((Get-Date) - $t0).TotalSeconds, 2) }
}

function Traeger-Zeile([string] $text, [string] $marke, [string] $traeger) {
    foreach ($z in ($text -split "`n")) {
        $zt = $z.Trim()
        if ($zt.StartsWith($marke) -and $zt.Contains($traeger)) { return $zt }
    }
    return ''
}

function Exit-Passt($code, $erwartet) {
    if ($null -eq $erwartet) { return $code -ne 0 }
    return $code -eq $erwartet
}

# ── Die Quelldateien ──────────────────────────────────────────────────────
$soakPy     = Join-Path $Wurzel 'tools\eq-copilot\pruefe_session_soak.py'
$truePeakH  = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\TruePeak.h'
$erzeuger   = Join-Path $Wurzel 'tools\eq-copilot\erzeuge_p4_korpus.py'
$korpusMan  = Join-Path $Wurzel 'eq-copilot\fixtures\p4-korpus\MANIFEST.json'
$korpusRef  = Join-Path $Wurzel 'eq-copilot\fixtures\p4-korpus\referenz.json'
$sonde013   = Join-Path $Wurzel 'docs\beweise\SONDE-013.md'
$manifest   = Join-Path $Wurzel 'docs\beweise\NAK-283.md'
$gesundheit = Join-Path $Wurzel 'tools\plan\gesundheit.py'

# Laeufe, die mehrfach vorkommen
function Selbsttest-Lauf([string] $zeile, [string] $traeger, [string] $datei) {
    return @{ Zeile = $zeile; Art = 'py'; Skript = $soak; Arg = @('--selbsttest'); RotExit = 2; GruenExit = 0
              RotMarke = 'ROT'; GruenMarke = 'ok'; Traeger = $traeger; Datei = $datei }
}
function B17-Lauf([string] $zeile, [string] $traeger, [string] $datei) {
    return @{ Zeile = $zeile; Art = 'exe'; Ziel = 'EqCopSonde013TruePeakGoldenTest'; Arg = @(); RotExit = 1; GruenExit = 0
              RotMarke = '[ROT]'; GruenMarke = '[ok]'; Traeger = $traeger; Datei = $datei }
}
function Riegel-Lauf([string] $zeile, [string[]] $rot, [string[]] $gruen, [string] $traeger, [string] $datei) {
    return @{ Zeile = $zeile; Art = 'py'; Skript = $riegel; ArgRot = $rot; ArgGruen = $gruen; RotExit = 1; GruenExit = 0
              RotMarke = 'FEHLER'; GruenMarke = 'ok'; Traeger = $traeger; Datei = $datei }
}

$mut62Aenderung = @{ Datei = $soakPy
    Alt = @('    return {"rss_bytes": None, "rss_gueltig": False, "rss_fehler": grund}')
    Neu = @('    return {"rss_bytes": 0, "rss_gueltig": True}   # MUTATION NAK-283 Rotbeweis M-62/M-70: der Fehlerwert wieder als Zahl 0') }

$mut66Aenderung = @{ Datei = $erzeuger
    Alt = @(
        '         "hinweis": "EBU Tech 3341 Faelle 15-19: fs/4, fs/6, fs/8 bei 0,50 und 1,41 FFS"},',
        '        # NAK-283 N01 (13.09.2026): die transienten Faelle als EIGENER Eintrag',
        '        # (Manifest §8.1 Feinheit 15) - eine andere Referenz (im Test gerechnet',
        '        # und validiert statt analytisch) und eine zweite Toleranz. Die',
        '        # Konfidenz liegt deshalb eine Stufe unter 15-19.',
        '        {"bein": "B17", "fall": "true_peak_matches_ebu_transient_set",',
        '         "quelle": q(PLUGIN + "Sonde013TruePeakGoldenTest.cpp",',
        '                     "true_peak_matches_ebu_transient_set"),',
        '         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.96,',
        '         "hinweis": ("EBU Tech 3341 Faelle 20-23: transient, vier Abtastversaetze, "',
        '                     "±0,1 dB gegen validierte Referenz plus Normtoleranz +0,2/−0,4 dBTP")},')
    Neu = @('         "hinweis": "EBU Tech 3341 Faelle 15-23: fs/4, fs/6, fs/8 bei 0,50 und 1,41 FFS"},   # MUTATION NAK-283 Rotbeweis M-66: zweiter Eintrag weg, hinweis des ersten auf 15-23') }

# ── Die Mutationen ────────────────────────────────────────────────────────
$mutationen = @(
    # ── F13: pruefe_session_soak.py --selbsttest (neues Bein A34) ──────────
    [pscustomobject]@{
        Kennung = 'MUT-61'; Zeilen = 'M-61'
        Was     = 'pruefe_session_soak.py gueltig_gemessen: die Erfolgsbedingung entfernen - die Budgetrechnung laeuft, obwohl tragende Messungen fehlen'
        Aenderungen = @(@{ Datei = $soakPy
            Alt = @(
                '        if tragend_ohne:',
                '            messung_fehlt("S07",',
                '                          f"{name}: Messung fehlt - das Working Set ist nicht gueltig "',
                '                          f"gemessen, kein Budget wird gerechnet",',
                '                          "; ".join(f"minute {p.get(''minute'')} bei {p.get(''sekunden'')} s: "',
                '                                    f"{p.get(''rss_fehler'') or ''ohne Grund''}"',
                '                                    for p in tragend_ohne))',
                '            return False',
                '        return True')
            Neu = @('        return True   # MUTATION NAK-283 Rotbeweis M-61: Erfolgsbedingung entfernt') })
        Ziele   = @()
        Laeufe  = @(Selbsttest-Lauf 'M-61' 'fehlende_rss_messung_ist_kein_pass' 'NAK-283-rot-M-61-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-62'; Zeilen = 'M-62, M-70'
        Was     = 'pruefe_session_soak.py _fehlmessung: den Fehlerwert wieder als Zahl 0 kodieren - ein nicht abfragbarer Prozess sieht aus wie gemessene 0 Bytes (Stand am Basis-SHA)'
        Aenderungen = @($mut62Aenderung)
        Ziele   = @()
        Laeufe  = @(
            (Selbsttest-Lauf 'M-62' 'messfehler_und_nullmessung_sind_unterscheidbar' 'NAK-283-rot-M-62-etappe-6.txt'),
            (Riegel-Lauf 'M-70' @('M-70') @('M-70') 'P2' 'NAK-283-rot-M-70-etappe-6.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-63'; Zeilen = 'M-63'
        Was     = 'pruefe_session_soak.py gueltig_gemessen: das Gueltigkeitsmerkmal nur innerhalb des Neustartfensters pruefen'
        Aenderungen = @(@{ Datei = $soakPy
            Alt = @('                        if not p.get("im_neustartfenster") and merkmal(p) is False]')
            Neu = @('                        if p.get("im_neustartfenster") and merkmal(p) is False]   # MUTATION NAK-283 Rotbeweis M-63: nur im Fenster geprueft') })
        Ziele   = @()
        Laeufe  = @(Selbsttest-Lauf 'M-63' 'fehlerpunkt_ausserhalb_des_fensters_faellt' 'NAK-283-rot-M-63-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-64'; Zeilen = 'M-64 (Regressionswache)'
        Was     = 'pruefe_session_soak.py merkmal: eine Fassung, die das fehlende Merkmal als Messfehler wertet'
        Aenderungen = @(@{ Datei = $soakPy
            Alt = @('        return p.get("rss_gueltig")')
            Neu = @('        return p.get("rss_gueltig", False)   # MUTATION NAK-283 Rotbeweis M-64: fehlendes Merkmal gilt als Messfehler') })
        Ziele   = @()
        Laeufe  = @(Selbsttest-Lauf 'M-64' 'altbericht_ohne_merkmal_bleibt_auswertbar' 'NAK-283-rot-M-64-etappe-6.txt')
    }

    # ── N01: B17 EqCopSonde013TruePeakGoldenTest ───────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-65-MATRIX'; Zeilen = 'M-65 (Matrixmutation, beobachtet)'; NurBeobachtung = $true
        Was     = 'TruePeak.h: der 8-fach-Pfad auf 4-fach (Rotbeweisspalte von M-65 woertlich)'
        Aenderungen = @(@{ Datei = $truePeakH
            Alt = @('    static constexpr int kFaktor = 8;')
            Neu = @('    static constexpr int kFaktor = 4;   // MUTATION NAK-283 Rotbeweis M-65 (Matrix): 8-fach auf 4-fach') })
        Ziele   = @('EqCopSonde013TruePeakGoldenTest')
        Laeufe  = @(B17-Lauf 'M-65 Matrix' 'true_peak_matches_ebu_transient_set' 'NAK-283-rot-M-65-matrix-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-65b'; Zeilen = 'M-65'
        Was     = 'TruePeak.h: der Interpolator mit 12 statt 24 Taps je Phase - ein kuerzerer Filter, dessen Durchlass den transienten Anteil zwischen 0,4 und 0,5*fs nicht mehr traegt'
        Aenderungen = @(@{ Datei = $truePeakH
            Alt = @('    static constexpr int kTapsJePhase = 24;')
            Neu = @('    static constexpr int kTapsJePhase = 12;   // MUTATION NAK-283 Rotbeweis M-65b: kuerzerer Interpolator') })
        Ziele   = @('EqCopSonde013TruePeakGoldenTest')
        Laeufe  = @(B17-Lauf 'M-65' 'true_peak_matches_ebu_transient_set Testfall 22' 'NAK-283-rot-M-65-etappe-6.txt')
    }

    # ── N01: P4-Korpus ─────────────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-66'; Zeilen = 'M-66'
        Was     = 'erzeuge_p4_korpus.py: den zweiten Eintrag weglassen und nur den hinweis des ersten auf 15-23 aendern (Rotbeweisspalte woertlich); der committete Korpus bleibt'
        Aenderungen = @($mut66Aenderung)
        Ziele   = @()
        Laeufe  = @(
            @{ Zeile = 'M-66 A25'; Art = 'py'; Skript = 'tools\eq-copilot\erzeuge_p4_korpus.py'; Arg = @('--pruefen'); RotExit = 2; GruenExit = 0
               RotMarke = 'ROT:'; GruenMarke = 'bytegleich'; Traeger = 'nicht bytegleich: referenz.json'; GruenTraeger = ''
               Datei = 'NAK-283-rot-M-66-etappe-6.txt' },
            (Riegel-Lauf 'M-66 Riegel' @('M-66') @('M-66') 'K2 Neuerzeugung' 'NAK-283-rot-M-66-riegel-etappe-6.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-66b'; Zeilen = 'M-66 (Korpus ueber den mutierten Erzeuger neu geschrieben)'
        Was     = 'wie MUT-66, danach schreibt der mutierte Erzeuger den Korpus neu - A25, A26 und A27 sehen die Behauptung nicht, der Riegel M-66 sieht sie'
        Aenderungen = @($mut66Aenderung)
        Sicherung = @($korpusMan, $korpusRef)
        Vorlauf = @('tools\eq-copilot\erzeuge_p4_korpus.py')
        Ziele   = @()
        Laeufe  = @(
            (Riegel-Lauf 'M-66b Riegel' @('M-66') @('M-66') 'K2 committeter Korpus' 'NAK-283-rot-M-66b-etappe-6.txt'),
            @{ Zeile = 'M-66b A25'; Art = 'py'; Skript = 'tools\eq-copilot\erzeuge_p4_korpus.py'; Arg = @('--pruefen'); Beobachtung = $true
               RotExit = 0; GruenExit = 0; RotMarke = 'bytegleich'; GruenMarke = 'bytegleich'; Traeger = ''; Datei = 'NAK-283-rot-M-66b-A25-etappe-6.txt' },
            @{ Zeile = 'M-66b A26'; Art = 'py'; Skript = 'tools\eq-copilot\pruefe_p4_korpus.py'; Arg = @(); Beobachtung = $true
               RotExit = 0; GruenExit = 0; RotMarke = 'Exit-Gate P4'; GruenMarke = 'Exit-Gate P4'; Traeger = ''; Datei = 'NAK-283-rot-M-66b-A26-etappe-6.txt' },
            @{ Zeile = 'M-66b A27'; Art = 'py'; Skript = 'tools\eq-copilot\pruefe_p4_korpus.py'; Arg = @('--selbsttest'); Beobachtung = $true
               RotExit = 0; GruenExit = 0; RotMarke = 'A27:'; GruenMarke = 'A27:'; Traeger = ''; Datei = 'NAK-283-rot-M-66b-A27-etappe-6.txt' })
    }

    # ── N01: Behauptungen ──────────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-67'; Zeilen = 'M-67'
        Was     = 'SONDE-013.md M-02: die Statusspalte umschreiben statt die Nachweisspalte zu ergaenzen'
        Aenderungen = @(@{ Datei = $sonde013
            Alt = @('§49.3:3584. **BELEGT, gemessen** (Etappe C, 2026-09-04) — der 8-fach-Pfad besteht die Prüfung und bleibt |')
            Neu = @('§49.3:3584. **BELEGT, gemessen** (Etappe C, 2026-09-04; 15 bis 23 seit 13.09.2026) — der 8-fach-Pfad besteht die Prüfung und bleibt |') })
        Ziele   = @()
        Laeufe  = @(Riegel-Lauf 'M-67' @('M-67') @('M-67') 'R2 M-02' 'NAK-283-rot-M-67-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-67-BASIS'; Zeilen = 'M-67 (Basislauf)'
        Was     = 'keine Mutation: derselbe Riegel misst rot den Stand am Basis-SHA (git show), gruen den Arbeitsbaum'
        Aenderungen = @(); Ziele = @()
        Laeufe  = @(Riegel-Lauf 'M-67 Basis' @('M-67', '--revision', $basis) @('M-67') 'R4 M-02' 'NAK-283-rot-M-67-basis-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-68'; Zeilen = 'M-68'
        Was     = 'NAK-283.md Wortlaut des NAK-199-Nachtrags: die Zeile behaelt den alten Umfang 15 bis 19, obwohl der neue gemessen ist'
        Aenderungen = @(@{ Datei = $manifest
            Alt = @('**Nachtrag 13.09.2026 (NAK-283, N01):** True Peak jetzt über 15 bis 23 gedeckt')
            Neu = @('**Nachtrag 13.09.2026 (NAK-283, N01):** True Peak über 15 bis 19 gedeckt') })
        Ziele   = @()
        Laeufe  = @(Riegel-Lauf 'M-68' @('M-68') @('M-68') 'R3' 'NAK-283-rot-M-68-etappe-6.txt')
    }
    [pscustomobject]@{
        Kennung = 'MUT-68-REGISTER'; Zeilen = 'M-68 (Registerstand)'
        Was     = 'keine Mutation: derselbe Riegel misst rot die Registerzeile, wie sie heute steht (Nachtrag ist Dirigentensache), gruen die Fixfassung mit dem Wortlaut'
        Aenderungen = @(); Ziele = @()
        Laeufe  = @(Riegel-Lauf 'M-68 Register' @('M-68', '--registerstand') @('M-68') 'R3' 'NAK-283-rot-M-68-register-etappe-6.txt')
    }

    # ── NAK-249: gesundheit.py ─────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-69'; Zeilen = 'M-69'
        Was     = 'gesundheit.py quellorte: den Unterbaum state still aus dem gemessenen Umfang nehmen, ohne ihn auszunehmen'
        Aenderungen = @(@{ Datei = $gesundheit
            Alt = @('    return [e for e in inv if e["grund"] is None]')
            Neu = @('    return [e for e in inv if e["grund"] is None and not e["pfad"].endswith("/state")]   # MUTATION NAK-283 Rotbeweis M-69') })
        Ziele   = @()
        Laeufe  = @(
            @{ Zeile = 'M-69'; Art = 'py'; Skript = 'tools\plan\gesundheit.py'; Arg = @('--selbsttest'); RotExit = 2; GruenExit = 0
               RotMarke = 'ROT'; GruenMarke = 'ok'; Traeger = 'kein_quellbaum_bleibt_ungemessen: jede nicht ausgenommene Quelldatei'
               Datei = 'NAK-283-rot-M-69-etappe-6.txt' },
            @{ Zeile = 'M-69 A32'; Art = 'py'; Skript = 'tools\plan\gesundheit.py'; Arg = @('--mit-selbsttest'); RotExit = 2; GruenExit = 4
               RotMarke = 'ABBRUCH'; GruenMarke = 'URTEIL'; Traeger = 'Selbsttest rot'; GruenTraeger = 'GRENZE GERISSEN'
               Datei = 'NAK-283-rot-M-69-A32-etappe-6.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-69-BASIS'; Zeilen = 'M-69 (Basislauf)'
        Was     = 'keine Mutation: das Werkzeug des Basis-SHA laesst acht Unterbaeume ungemessen (rot), das Werkzeug des Arbeitsbaums keinen (gruen)'
        Aenderungen = @(); Ziele = @()
        Laeufe  = @(Riegel-Lauf 'M-69 Basis' @('basis-M-69', '--revision', $basis) @('basis-M-69') 'M-69' 'NAK-283-rot-M-69-basis-etappe-6.txt')
    }

    # ── F13-Basislaeufe und der Uebergang M-70 ────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-61-62-BASIS'; Zeilen = 'M-61, M-62 (Basislauf)'
        Was     = 'keine Mutation: das Soak-Orakel des Basis-SHA laesst 0,0 -> 0,0 MB bestehen und kodiert den Messfehler wie 0 Bytes (rot), das Orakel des Arbeitsbaums nicht (gruen)'
        Aenderungen = @(); Ziele = @()
        Laeufe  = @(
            (Riegel-Lauf 'M-61 Basis' @('basis-M-61-M-62', '--revision', $basis) @('basis-M-61-M-62') 'M-61' 'NAK-283-rot-M-61-basis-etappe-6.txt'),
            (Riegel-Lauf 'M-62 Basis' @('basis-M-61-M-62', '--revision', $basis) @('basis-M-61-M-62') 'M-62' 'NAK-283-rot-M-62-basis-etappe-6.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-70-BASIS'; Zeilen = 'M-70 (Basislaeufe)'
        Was     = 'keine Mutation: am Basis-SHA der Etappe 6 fehlt die Python-Haelfte, am Basis-SHA der Etappe 1 fehlen beide Haelften (rot); am Arbeitsbaum stehen beide (gruen)'
        Aenderungen = @(); Ziele = @()
        Laeufe  = @(
            (Riegel-Lauf 'M-70 Basis' @('M-70', '--revision', $basis) @('M-70') 'P1' 'NAK-283-rot-M-70-basis-etappe-6.txt'),
            (Riegel-Lauf 'M-70 Etappe 1' @('M-70', '--revision', $etappe1) @('M-70') 'C1' 'NAK-283-rot-M-70-etappe-1-basis-etappe-6.txt'))
    }
)

# ── Vorbau: B17 und B27 frisch, bevor irgendetwas mutiert ────────────────
$vorbau = Baue @('EqCopSonde013TruePeakGoldenTest', 'EqCopSonde014IntentTest')
if ($vorbau.ExitCode -ne 0) { throw "Vorbau fehlgeschlagen: $($vorbau.Text)" }

# ── Lauf ──────────────────────────────────────────────────────────────────
$bilanz = @()
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ($($m.Zeilen)) ===" -ForegroundColor Cyan

    $dateien = @(@($m.Aenderungen | ForEach-Object { $_.Datei }) + @($m.Sicherung) | Where-Object { $_ } | Select-Object -Unique)
    $sicherung = @{}
    $hashVorher = @{}
    foreach ($d in $dateien) { $sicherung[$d] = [System.IO.File]::ReadAllBytes($d); $hashVorher[$d] = Hash-Von $d }

    $rotLaeufe = @()
    $vorlaufText = ''
    $frischeRot = ''
    try {
        foreach ($a in $m.Aenderungen) { Mutiere $a.Datei $a.Alt $a.Neu }
        if ($m.Vorlauf) {
            $v = & py -3.13 (Join-Path $Wurzel $m.Vorlauf[0]) 2>&1
            $vorlaufText = "py -3.13 $($m.Vorlauf[0].Replace('\', '/')) -> Exit $($LASTEXITCODE): " + (($v | ForEach-Object { "$_" }) -join ' / ')
        }
        foreach ($d in $dateien) { (Get-Item $d).LastWriteTime = Get-Date }
        $bauRot = Baue $m.Ziele
        if ($bauRot.ExitCode -ne 0) { throw "Bau der mutierten Fassung fehlgeschlagen: $($bauRot.Text)" }
        $frischeRot = Pruefe-Frische $m.Ziele $dateien 'rot'
        foreach ($l in $m.Laeufe) {
            $r = Fahre $l 'rot'
            $tz = Traeger-Zeile $r.Text $l.RotMarke $l.Traeger
            $rotLaeufe += [pscustomobject]@{ Lauf = $l; Ergebnis = $r; Traeger = $tz }
            Write-Host ("  ROT   {0} -> Exit {1}, Traeger {2}" -f $l.Datei, $r.ExitCode, $(if ($tz) { 'gefallen' } else { 'NICHT gefallen' }))
        }
    }
    finally {
        foreach ($d in $dateien) { [System.IO.File]::WriteAllBytes($d, $sicherung[$d]) }
    }

    $hashNachher = @{}
    foreach ($d in $dateien) {
        $hashNachher[$d] = Hash-Von $d
        if ($hashNachher[$d] -ne $hashVorher[$d]) { throw "Ruecknahme NICHT bytegleich in $(Relativ $d)" }
        (Get-Item $d).LastWriteTime = Get-Date
    }
    $bauGruen = Baue $m.Ziele
    if ($bauGruen.ExitCode -ne 0) { throw "Bau der zurueckgespielten Fassung fehlgeschlagen: $($bauGruen.Text)" }
    $frischeGruen = Pruefe-Frische $m.Ziele $dateien 'gruen'

    foreach ($e in $rotLaeufe) {
        $l = $e.Lauf
        $g = Fahre $l 'gruen'
        $gruenTraeger = if ($l.ContainsKey('GruenTraeger')) { $l.GruenTraeger } else { $l.Traeger }
        $tg = Traeger-Zeile $g.Text $l.GruenMarke $gruenTraeger
        $beobachtung = [bool] $m.NurBeobachtung -or [bool] $l.Beobachtung
        Write-Host ("  GRUEN {0} -> Exit {1}, Traeger {2}" -f $l.Datei, $g.ExitCode, $(if ($tg) { 'ok' } else { 'FEHLT' }))
        $roteZeilen = @(($e.Ergebnis.Text -split "`n") | Where-Object { $_.Trim().StartsWith($l.RotMarke) } | ForEach-Object { $_.Trim() })
        $titel = "NAK-283 Etappe 6 - Rotbeweis $($m.Kennung) fuer $($l.Zeile)"
        if ($beobachtung) { $titel += ' (BEOBACHTUNG, kein Rotbeweis)' }
        $kopf = @(
            $titel,
            "Zeitpunkt:  $(Get-Date -Format o)",
            "HEAD:       $(git -C $Wurzel rev-parse HEAD)",
            "Mutation:   $($m.Was)")
        if ($dateien.Count -eq 0) { $kopf += 'Datei:      (keine Mutation)' }
        foreach ($d in $dateien) {
            $kopf += "Datei:      $(Relativ $d)"
            $kopf += "  SHA-256 vorher:  $($hashVorher[$d])"
            $kopf += "  SHA-256 nachher: $($hashNachher[$d]) (bytegleiche Ruecknahme: $($hashVorher[$d] -eq $hashNachher[$d]))"
        }
        if ($vorlaufText) { $kopf += "Vorlauf:    $vorlaufText" }
        $kopf += @(
            "Befehl:     rot '$($e.Ergebnis.Befehl)' / gruen '$($g.Befehl)'",
            "Neubau:     rot $(if ($m.Ziele.Count) { "$($m.Ziele -join ' ') Exit $($bauRot.ExitCode), frisch: $frischeRot" } else { '(kein Bauziel)' }) / gruen $(if ($m.Ziele.Count) { "Exit $($bauGruen.ExitCode), frisch: $frischeGruen" } else { '(kein Bauziel)' })",
            "Traeger:    rot '$($l.RotMarke)' + '$($l.Traeger)' / gruen '$($l.GruenMarke)' + '$gruenTraeger'",
            "Erwartet:   rot Exit $(if ($null -eq $l.RotExit) { '!= 0' } else { $l.RotExit }) / gruen Exit $($l.GruenExit)",
            "Urteil:     ROT Exit $($e.Ergebnis.ExitCode) ($($e.Ergebnis.Sekunden) s) / GRUEN Exit $($g.ExitCode) ($($g.Sekunden) s)",
            "Traegerzeile rot:   $($e.Traeger)",
            "Traegerzeile gruen: $tg",
            "Zeilen mit roter Marke im roten Lauf: $($roteZeilen.Count)")
        foreach ($z in $roteZeilen) { $kopf += "  $z" }
        $kopf += @(
            "",
            "--- MUTIERT / ROT (erwartet: rot) -----------------------------------",
            $e.Ergebnis.Text,
            "",
            "--- ZURUECKGESPIELT / GRUEN (erwartet: gruen) -----------------------",
            $g.Text)
        [System.IO.File]::WriteAllLines((Join-Path $rohOrdner $l.Datei), $kopf, $utf8)
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Zeile = $l.Zeile; Datei = $l.Datei; Beobachtung = $beobachtung
            Rot = $e.Ergebnis.ExitCode; Gruen = $g.ExitCode
            RotPasst = (Exit-Passt $e.Ergebnis.ExitCode $l.RotExit); GruenPasst = (Exit-Passt $g.ExitCode $l.GruenExit)
            TraegerRot = [bool] $e.Traeger; TraegerGruen = [bool] $tg
            HashGleich = (@($dateien | Where-Object { $hashVorher[$_] -ne $hashNachher[$_] }).Count -eq 0)
            RoteZeilen = $roteZeilen.Count
        }
    }
}

$zeilen = @('NAK-283 Etappe 6 - Rotbilanz', "Zeitpunkt: $(Get-Date -Format o)", "HEAD: $(git -C $Wurzel rev-parse HEAD)", '')
foreach ($b in $bilanz) {
    if ($b.Beobachtung) {
        $zeilen += ("{0,-16} {1,-14} BEOBACHTUNG rot-Lauf Exit {2} (Traeger gefallen {3}, Zeilen mit roter Marke {4}) / gruen Exit {5}, Hash gleich {6}  {7}" -f `
            $b.Kennung, $b.Zeile, $b.Rot, $b.TraegerRot, $b.RoteZeilen, $b.Gruen, $b.HashGleich, $b.Datei)
        continue
    }
    $ok = $b.RotPasst -and $b.GruenPasst -and $b.TraegerRot -and $b.TraegerGruen -and $b.HashGleich
    $zeilen += ("{0,-16} {1,-14} rot Exit {2} (Traeger {3}) / gruen Exit {4} (Traeger {5}), Hash gleich {6}  {7}  {8}" -f `
        $b.Kennung, $b.Zeile, $b.Rot, $b.TraegerRot, $b.Gruen, $b.TraegerGruen, $b.HashGleich, $b.Datei, $(if ($ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }))
}
$gezaehlt = @($bilanz | Where-Object { -not $_.Beobachtung })
$schlecht = @($gezaehlt | Where-Object { -not ($_.RotPasst -and $_.GruenPasst -and $_.TraegerRot -and $_.TraegerGruen -and $_.HashGleich) })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Rotbeweisen bestanden (Exit wie erwartet, rot mit Traeger, gruen mit Traeger, Ruecknahme bytegleich); {2} Beobachtungslaeufe" -f `
    ($gezaehlt.Count - $schlecht.Count), $gezaehlt.Count, ($bilanz.Count - $gezaehlt.Count))
if (-not $Nur) { [System.IO.File]::WriteAllLines((Join-Path $rohOrdner 'NAK-283-etappe-6-rotbilanz.txt'), $zeilen, $utf8) }
$zeilen | ForEach-Object { Write-Host $_ }
if ($schlecht.Count -gt 0) { exit 1 }
exit 0
