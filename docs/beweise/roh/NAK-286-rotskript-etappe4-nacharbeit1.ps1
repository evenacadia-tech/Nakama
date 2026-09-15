<#
NAK-286 Etappe 4, Nacharbeit 1 - Rotlaeufe vor der Aenderung, Rotbeweise und Gegenproben (Manifest
docs/beweise/NAK-286.md §40; §5.1, Regeln P-16, P-19, P-20, Ursache m). Muster: NAK-286-rotskript-etappe4.ps1.

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4-nacharbeit1.ps1 -Art vorher       Rotlaeufe vor der Aenderung
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4-nacharbeit1.ps1 -Art nach         Rotbeweise nach dem Bau
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4-nacharbeit1.ps1 -Art gegenprobe   Mutation ohne Traeger
  ... -Nur M-21,M-03   nur diese Zeilen        ... -NurPruefen   nur Traegertexte zaehlen

Je Eintrag, in dieser Reihenfolge (§5.1; PowerShell und Python ohne den LastWriteTime-Schritt):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot4-nach1).
  2. Mutation genau an der Traegerzeile, jeder Alttext genau einmal (P-20: haelt eine redundante Schranke die
     Zusage, gehoert sie zur Mutation). Ein Rotlauf vor der Aenderung faehrt den neuen Fall gegen den noch
     unveraenderten Traeger und mutiert nichts.
  3. Das Bein einzeln, mit Obergrenze: A35 als einzelner Selbsttestfall (laufzeit.ps1 -Selbsttest -Nur,
     szenario.py --selbsttest --nur). Nach Ablauf der Obergrenze wird der Prozessbaum beendet (taskkill /T /F);
     das Ende an der Obergrenze steht in der Rohdatei. Erwartung "rot": Exit ungleich 0 und mindestens die
     verlangte Zahl Zeilen, die den Traeger nennen (Muster des Traegers, nie der Meldung). Erwartung "ohne
     Traeger": Exit ungleich 0 und keine Zeile mit dem Traeger (Nachweis, dass der Fall den Traeger nicht sieht
     oder die Mutation ihn nicht falsifiziert).
  4. Bytegleiche Ruecknahme aus den gesicherten Bytes (Schreiben mit Wiederholung), SHA-256 gleich Schritt 1,
     derselbe Lauf gruen.
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-<nn>-nach1.txt je Zeile, je Eintrag ein angehaengter Abschnitt.
Der Lauf startet kein FL, installiert nichts und faehrt keinen Laufzeitlauf: jedes Bein laeuft gegen Attrappen.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen, [ValidateSet('vorher', 'nach', 'gegenprobe')] [string] $Art = 'nach',
      [int] $ObergrenzeSekunden = 240)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$tmp = Join-Path $env:TEMP 'nak286-rot4-nach1'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
$env:PYTHONIOENCODING = 'utf-8'
$env:PYTHONDONTWRITEBYTECODE = '1'

$LZ = 'tools/fl/laufzeit.ps1'
$SZ = 'tools/fl/szenario.py'
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
# Defekt 1 (P-19, M-21): der neue Fall mit blockierender Erfassung gegen den unveraenderten Runner (Erfassung in-process).
$eintraege.Add((Eintrag 'vorher' 'M-21' '(P-19)' 'neuer Fall fenster_frist_erzwungen gegen die unveraenderte Erfassung in-process' 'sz:fenster_frist_erzwungen' '\[ROT\] M-21 fenster_frist_erzwungen:.*Frist nicht erzwungen: lokal\.fenster kehrte nach' 'NAK-286-rot-M-21-nach1.txt' 'rot' @($SZ) @()))

# Defekt 2 (P-16, M-71): der zeilengenaue Fall gegen die unveraenderte stellen_zeile.
$eintraege.Add((Eintrag 'vorher' 'M-71' '(P-16)' 'Fall stelle_zwei_durchlaeufe prueft die Rohzeile gegen die unveraenderte stellen_zeile' 'sz:stelle_zwei_durchlaeufe' '\[ROT\] M-70 stelle_zwei_durchlaeufe:.*Rohzeile ohne durchlauf und kombinationen' 'NAK-286-rot-M-71-nach1.txt' 'rot' @($SZ) @()))

# Ursache m (iii), F-23 und F-22: Lage der Stelle im Host mit v in jeder Rohzeile, gegen die unveraenderte stellen_zeile.
$eintraege.Add((Eintrag 'vorher' 'M-71' '(Lage)' 'neuer Fall rohzeile_lage_im_host gegen die unveraenderte stellen_zeile' 'sz:rohzeile_lage_im_host' '\[ROT\] M-71 rohzeile_lage_im_host:.*Rohzeile ohne Lage im Host mit v.*Rohzeile ohne Vermerk v unbekannt' 'NAK-286-rot-M-71-nach1.txt' 'rot' @($SZ) @()))

# Ursache m (iii), F-28: je Referenzausschnitt eine Rohzeile mit Frames, K, v und SHA-256, gegen das unveraenderte lokal.umlauf.
$eintraege.Add((Eintrag 'vorher' 'M-67' '(Referenzausschnitt)' 'neuer Fall referenzausschnitt_rohzeile gegen das unveraenderte lokal.umlauf' 'sz:referenzausschnitt_rohzeile' '\[ROT\] M-67 referenzausschnitt_rohzeile:.*Referenzausschnitt ohne Rohzeile mit Frames, K, v und SHA-256' 'NAK-286-rot-M-67-nach1.txt' 'rot' @($SZ) @()))

# Defekt 3 (P-20, M-03): die vollstaendige Mutation samt redundanter Schranke gegen den unveraenderten Selbsttest.
# Erwartet: rot, aber keine Zeile mit dem Traeger "Renderprozess trotz fremdem FL gestartet" - die alte Sammelpruefung
# nennt start /R nicht als eigenen Traeger.
$M03Render = @(
    (Ersetzung $LZ @'
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen vor dem Render: '$($fremd[0].Titel)'" }
'@ @'
    # ROTBEWEIS M-03 (Render): Pruefung vor dem Render entfernt
'@),
    (Ersetzung $LZ @'
    $rest = @(FL-Instanzen)
'@ @'
    $rest = @(FL-Instanzen | Where-Object { -not (Ist-Fremd $_) }) # ROTBEWEIS M-03 (Render): redundante Schranke entfernt
'@))
$eintraege.Add((Eintrag 'vorher' 'M-03' '(Render)' 'vollstaendige Mutation (Pruefung vor dem Render und Schranke $rest) gegen den unveraenderten Selbsttest' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden:.*Renderprozess trotz fremdem FL gestartet' 'NAK-286-rot-M-03-nach1.txt' 'ohne Traeger' @($LZ) $M03Render))

# ================================================================ Rotbeweise nach dem Bau
# M-21 (P-19): die Frist an ihrer timeout-Uebergabe entfernt - die blockierende Erfassung kehrt erst mit ihrem Ende zurueck.
$eintraege.Add((Eintrag 'nach' 'M-21' '(P-19)' 'Frist der Fenstererfassung an der timeout-Uebergabe entfernt (frist=None)' 'sz:fenster_frist_erzwungen' '\[ROT\] M-21 fenster_frist_erzwungen:.*Frist nicht erzwungen: lokal\.fenster kehrte nach' 'NAK-286-rot-M-21-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                                               frist=FENSTER_FRIST_S, bezeichnung=f"Fenstererfassung {ziel} (PID {pid})")
'@ @'
                                               frist=None, bezeichnung=f"Fenstererfassung {ziel} (PID {pid})")  # ROTBEWEIS M-21 (P-19)
'@))))

# M-71 (P-16, F-23, F-22): an der Rohzeile.
$eintraege.Add((Eintrag 'nach' 'M-71' '(3)' 'Spalten durchlauf und kombinationen aus stellen_zeile entfernt' 'sz:stelle_zwei_durchlaeufe' '\[ROT\] M-70 stelle_zwei_durchlaeufe:.*Rohzeile ohne durchlauf und kombinationen ihres Durchlaufs' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    return (f"| {stelle_id} | {rolle} | {durchlauf} | {kombinationen} | `{antwort.kennung[:8]}` | {_zelle(urteil)} | "
'@ @'
    return (f"| {stelle_id} | {rolle} | `{antwort.kennung[:8]}` | {_zelle(urteil)} | "  # ROTBEWEIS M-71 (3): ohne durchlauf und kombinationen
'@))))

$eintraege.Add((Eintrag 'nach' 'M-71' '(6)' 'Differenzzeile ohne durchlauf und kombinationen' 'sz:stelle_zwei_durchlaeufe' '\[ROT\] M-70 stelle_zwei_durchlaeufe:.*Differenzzeile ohne durchlauf und kombinationen' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen.append(f"- Stelle {sid} {rolle} (durchlauf {durchlauf}, kombinationen {kombinationen_je_rolle.get(rolle, '0/0')}): "
'@ @'
        zeilen.append(f"- Stelle {sid} {rolle}: "  # ROTBEWEIS M-71 (6): Differenzzeile ohne durchlauf und kombinationen
'@))))

$eintraege.Add((Eintrag 'nach' 'M-71' '(4)' 'Lage im Host aus stellen_zeile entfernt' 'sz:rohzeile_lage_im_host' '\[ROT\] M-71 rohzeile_lage_im_host:.*Rohzeile ohne Lage im Host mit v' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            f"{werte} | {_zelle(lage)} | {klammer} |")
'@ @'
            f"{werte} | {klammer} |")  # ROTBEWEIS M-71 (4): ohne Lage im Host
'@))))

$eintraege.Add((Eintrag 'nach' 'M-71' '(5)' 'Vermerk v unbekannt in lage_im_host entfernt (ohne Nulltesturteil v = 0)' 'sz:rohzeile_lage_im_host' '\[ROT\] M-71 rohzeile_lage_im_host:.*Rohzeile ohne Vermerk v unbekannt' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    return f"[{a_frames}, {b_frames}) " + (f"v = {v}" if v_bekannt else "v unbekannt")
'@ @'
    return f"[{a_frames}, {b_frames}) " + f"v = {v}"  # ROTBEWEIS M-71 (5): ohne Vermerk v unbekannt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-71' '(1)' 'roh schreibt null als 0 (Fall mit neuer Spaltenfolge)' 'sz:roh_schreibt_null_als_null' '\[ROT\] M-71 roh_schreibt_null_als_null:.*null nicht als null geschrieben' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    werte = " | ".join(_zelle(_json(antwort.wert(f"frame.{f}"))) for f in F23_FELDER)
'@ @'
    werte = " | ".join(_zelle(_json(0 if antwort.wert(f"frame.{f}") is None else antwort.wert(f"frame.{f}"))) for f in F23_FELDER)  # ROTBEWEIS M-71 (1)
'@))))

# M-67 (F-28): Referenzausschnitt ohne K und v in seiner Rohzeile.
$eintraege.Add((Eintrag 'nach' 'M-67' '(9)' 'K und v aus der Rohzeile des Referenzausschnitts entfernt' 'sz:referenzausschnitt_rohzeile' '\[ROT\] M-67 referenzausschnitt_rohzeile:.*Referenzausschnitt ohne Rohzeile mit Frames, K, v und SHA-256' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                          f"K {anker[a['rolle']].get('k')}, v {v}, SHA-256 {x.get('sha256')}, Datei {x.get('pfad')}")
'@ @'
                          f"SHA-256 {x.get('sha256')}, Datei {x.get('pfad')}")  # ROTBEWEIS M-67 (9): ohne K und v
'@))))

# M-56 (Ursache m ii): PID-Filter entfernt - die Antwort der fremden PID wird gewertet (Rolle ohne Antwort der Diagnose-PID).
$eintraege.Add((Eintrag 'nach' 'M-56' '' 'PID-Filter entfernt' 'sz:antworten_auswahl' '\[ROT\] M-56 antworten_auswahl:.*fremde Antwort gewertet: gen <- [0-9a-f]{32}\.gen\.9999\.' 'NAK-286-rot-M-56-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            if pid not in erwartet[rolle]:
                aus.fremd.append((name, f"fremd: PID {pid} nicht erwartet {sorted(erwartet[rolle])}"))
                continue
'@ @'
            # ROTBEWEIS M-56: PID-Filter entfernt
'@))))

# M-70 (1) bis (7): erneut, weil messe_stelle, stellen_durchlauf und die Hilfen der Faelle geaendert sind; Muster am Traeger.
$eintraege.Add((Eintrag 'nach' 'M-70' '(1)' 'Zaehlung nach der Positionsklammer statt nach dem Materialausschnitt' 'sz:stelle_materialausschnitt' "\[ROT\] M-70 stelle_materialausschnitt:.*\(1\) Rahmen aus der Zeit vor dem Start bei Klammer in der Stelle: Zaehlung 'gezaehlt'" 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        elif not (a_frames <= start and start + anzahl <= b_frames):
'@ @'
        elif not (anfrage.p_vor is not None and a_frames <= round(anfrage.p_vor * RATE / 1000) <= b_frames):  # ROTBEWEIS M-70 (1)
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(2)' 'Stelle ohne Versatz in den Host gelegt' 'sz:stelle_materialausschnitt' '\[ROT\] M-70 stelle_materialausschnitt:.*\(2\) v = \+75 600: setPosition \[\{.position.: 0' 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        a = round(float(stelle["von_s"]) * RATE) + v
        b = round(float(stelle["bis_s"]) * RATE) + v
'@ @'
        a = round(float(stelle["von_s"]) * RATE)  # ROTBEWEIS M-70 (2): ohne Versatz
        b = round(float(stelle["bis_s"]) * RATE)
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(3)' 'null als 0 gelesen (Zaehlung und Ausschnitt der Differenzen)' 'sz:stelle_materialausschnitt' "\[ROT\] M-70 stelle_materialausschnitt:.*\(3\) projekt_sample_start null: Zaehlung 'gezaehlt'" 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        spielt = antwort.wert("frame.spielt")
'@ @'
        spielt = antwort.wert("frame.spielt")
        start = 0 if start is None else start  # ROTBEWEIS M-70 (3): null als 0
'@),
    (Ersetzung $SZ @'
        start, anzahl = antwort.wert("frame.projekt_sample_start"), antwort.wert("frame.sample_count")
        return [start, start + anzahl]
'@ @'
        start, anzahl = antwort.wert("frame.projekt_sample_start") or 0, antwort.wert("frame.sample_count")  # ROTBEWEIS M-70 (3)
        return [start, start + anzahl]
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(4)' 'Ende der Anfragen entfernt' 'sz:stelle_materialausschnitt' '\[ROT\] M-70 stelle_materialausschnitt:.*\(4\) Position steht nach dem Start: \d+ Anfragen' 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            if vor.ende:
                ende = vor.ende
                break
'@ @'
            pass  # ROTBEWEIS M-70 (4): Ende der Anfragen entfernt
'@),
    (Ersetzung $SZ @'
        if nach.ende:
            ende = nach.ende
            break
        if all(am_stellenende(anfragen, r, b_frames) for r in rollen):
'@ @'
        if all(am_stellenende(anfragen, r, b_frames) for r in rollen):
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(5)' 'Zeitplan ohne Bewertung: der erste Kandidat wird genommen' 'sz:stelle_zeitplan' '\[ROT\] M-70 stelle_zeitplan:.*im mittleren Fall' 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            if bester is None or (tragend, marge) > (bester[3], bester[2]):
'@ @'
            if bester is None:  # ROTBEWEIS M-70 (5): ohne Bewertung
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(6)' 'getrennte Durchlaeufe je Rolle entfernt: immer eine gemeinsame Anfragefolge' 'sz:stelle_zwei_durchlaeufe' '\[ROT\] M-70 stelle_zwei_durchlaeufe:.*Positionierungen \[.*\] statt zweimal 42 462 ms' 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    getrennt = len(rollen) > 1 and gemeinsam is not None and gemeinsam[3] < gemeinsam[4]
'@ @'
    getrennt = False  # ROTBEWEIS M-70 (6): immer gemeinsam
'@))))

$eintraege.Add((Eintrag 'nach' 'M-70' '(7)' 'erste Anfrage auch waehrend transport.start blockiert zugelassen' 'sz:stelle_zeitplan' '\[ROT\] M-70 stelle_zeitplan:.*waehrend transport\.start blockiert' 'NAK-286-rot-M-70-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            if t1 < jetzt + 0.1 or start - 0.02 <= t1 < start + 2.0 * befehl_s + 0.05:
'@ @'
            if t1 < jetzt + 0.1:  # ROTBEWEIS M-70 (7): Startfenster nicht ausgeschlossen
'@))))

# M-03 (P-20): Traeger der drei Haelften (Installation, Render, FL-Start) im Protokoll der Attrappen.
$eintraege.Add((Eintrag 'nach' 'M-03' '(Render)' 'Pruefung vor dem Render samt redundanter Schranke $rest entfernt: die Attrappe startet FL64 /R' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden:.*\[fremdes FL vor dem Render\] Renderprozess trotz fremdem FL gestartet' 'NAK-286-rot-M-03-nach1.txt' 'rot' @() $M03Render))

$eintraege.Add((Eintrag 'nach' 'M-03' '(Installation)' 'Pruefung auf ein fremdes FL vor der Installation entfernt' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden:.*\[fremdes FL vor der Installation\] Installation trotz fremdem FL gestartet' 'NAK-286-rot-M-03-nach1.txt' 'rot' @() @(
    (Ersetzung $LZ @'
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { return @{ Ok = $false; Ueberspringen = $true; Grund = "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" } }
'@ @'
    # ROTBEWEIS M-03 (Installation): Pruefung vor der Installation entfernt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-03' '(FL-Start)' 'Pruefung auf ein fremdes FL vor dem FL-Start entfernt' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden:.*\[fremdes FL vor dem FL-Start\] FL-Start trotz fremdem FL' 'NAK-286-rot-M-03-nach1.txt' 'rot' @() @(
    (Ersetzung $LZ @'
            if ($fremd.Count -gt 0 -and $laufend.Count -eq 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" }
'@ @'
            # ROTBEWEIS M-03 (FL-Start): Pruefung vor dem FL-Start entfernt
'@))))

# Ursache m (iii): zeilengebundene Rohzusagen ohne Zeilentest (F-23, F-24, F-28, M-11), je an der Zeile gebrochen.
$eintraege.Add((Eintrag 'nach' 'M-71' '(7)' 'Positionsklammer aus der Rohzeile der Stelle entfernt' 'sz:stelle_klammer_und_differenzen' '\[ROT\] M-71 stelle_klammer_und_differenzen:.*Rohzeile ohne Positionsklammer' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    klammer = f"{anfrage.p_vor} | {anfrage.p_nach}" if anfrage else "— | —"
'@ @'
    klammer = "— | —"  # ROTBEWEIS M-71 (7): ohne Positionsklammer
'@))))

$eintraege.Add((Eintrag 'nach' 'M-71' '(8)' 'Differenzen aus der Differenzzeile der Stelle entfernt' 'sz:stelle_klammer_und_differenzen' '\[ROT\] M-71 stelle_klammer_und_differenzen:.*Differenzzeile ohne Zahl der Antworten, Differenzen und Ausschnitte' 'NAK-286-rot-M-71-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                      f"{_json(delta)}{vermerk}")
'@ @'
                      f"{vermerk}")  # ROTBEWEIS M-71 (8): Differenzzeile ohne Differenzen
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(10)' 'Positionsklammer aus der Zeile je Anfrage von lokal.umlauf entfernt' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Anfrage ohne Rohzeile mit Positionsklammer' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen.append(f"| {nr} | `{anfrage.kennung[:8]}` | {anfrage.p_vor} | {anfrage.p_nach} | "
'@ @'
        zeilen.append(f"| {nr} | `{anfrage.kennung[:8]}` | "  # ROTBEWEIS M-67 (10): ohne Positionsklammer
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(11)' 'Anker aus der Zeile der gewerteten Antwort entfernt' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Anker ohne Rohzeile mit n_L, E, Fortlaufzaehlern und Hostblocklaenge' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                      f"Anker {_json(anker[rolle])}")
'@ @'
                      f"Anker")  # ROTBEWEIS M-67 (11): ohne Anker
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(12)' 'Dauer aus der Rechnungszeile entfernt' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Rechnung ohne ihre Dauer in der Rohzeile' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen.append(f"- Rechnung F-28 ({lauf.umg.jetzt() - t0:.1f} s): `{_zelle(_json(rechnung))}`")
'@ @'
        zeilen.append(f"- Rechnung F-28: `{_zelle(_json(rechnung))}`")  # ROTBEWEIS M-67 (12): ohne Dauer
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(13)' 'Rechnungszeile wieder auf 8 000 Zeichen gekappt' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Rechnung nicht vollstaendig in der Rohzeile' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen.append(f"- Rechnung F-28 ({lauf.umg.jetzt() - t0:.1f} s): `{_zelle(_json(rechnung))}`")
'@ @'
        zeilen.append(f"- Rechnung F-28 ({lauf.umg.jetzt() - t0:.1f} s): `{_zelle(_kompakt(rechnung, 8000))}`")  # ROTBEWEIS M-67 (13): gekappt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(14)' 'Zeilen der Baender nicht geschrieben' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Band ohne Rohzeile mit Wert und Status' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    zeilen += [f"- Band {e['kurz']} {e['rolle']}: {_json(e)}" for e in ergebnisse]
'@ @'
    zeilen += []  # ROTBEWEIS M-67 (14): ohne Zeilen der Baender
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(15)' 'Zeilen der Rohfelder nicht geschrieben' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Rohfeld ohne Rohzeile' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
            zeilen.append(f"- {rolle} roh `{feld}` = {_json([w for _, w in werte_an_pfad(antwort.umschlag, feld) if w is not _FEHLT])}")
'@ @'
            pass  # ROTBEWEIS M-67 (15): ohne Zeilen der Rohfelder
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(16)' 'Zeile Rohvergleich Runde 01 nicht geschrieben' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*Rohvergleich Runde 01 ohne Rohzeile' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen.append(f"- Rohvergleich Runde 01 (nur daneben, F-28): {_json(vergleich['rohvergleich_runde01'])}")
'@ @'
        pass  # ROTBEWEIS M-67 (16): ohne Rohvergleich
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(17)' 'Zeilen U_unten roh der Vorbedingung nicht geschrieben' 'sz:umlauf_rohzeilen' '\[ROT\] M-67 umlauf_rohzeilen:.*U_unten ohne Rohzeile' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
        zeilen += [f"- {rolle}: {x}" for x in roh]
'@ @'
        zeilen += []  # ROTBEWEIS M-67 (17): ohne Zeilen U_unten
'@))))

$eintraege.Add((Eintrag 'nach' 'M-68' '(5)' 'low_frac von analyze() aus dem Eintrag des Referenzausschnitts entfernt' 'sz:rechnung_rohwerte' '\[ROT\] M-68 rechnung_rohwerte:.*Referenzausschnitt ohne low_frac von analyze\(\) neben low_frac_kanal' 'NAK-286-rot-M-68-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                       "corr": float(a["corr"]), "low_frac": float(a["low_frac"]), "low_frac_kanal": low_frac_kanal(teil),
'@ @'
                       "corr": float(a["corr"]), "low_frac_kanal": low_frac_kanal(teil),  # ROTBEWEIS M-68 (5): ohne low_frac von analyze()
'@))))

$eintraege.Add((Eintrag 'nach' 'M-67' '(18)' 'Stichprobenspitze je Kanal aus dem Eintrag des Referenzausschnitts entfernt (True Peak, neben R_TP)' 'sz:rechnung_rohwerte' '\[ROT\] M-68 rechnung_rohwerte:.*Referenzausschnitt ohne Stichprobenspitze je Kanal neben R_TP' 'NAK-286-rot-M-67-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                       "stichprobenspitze": [max(abs(int(teil[:, c].max())), abs(int(teil[:, c].min()))) / (1 << 23)
                                             for c in range(2)] if teil.size else [0.0, 0.0],
'@ @'
                       # ROTBEWEIS M-67 (18): ohne Stichprobenspitze
'@))))

$eintraege.Add((Eintrag 'nach' 'M-11' '(Kopfzeile Exit 5)' 'Szenario-Exit 5 aus der Liste der Szenarioexits der Kopfzeile entfernt' 'ps:kopfzeile_und_protokoll' '\[ROT\] M-11 kopfzeile_und_protokoll:.*Kopfzeile ohne Szenario-Exit 5 in der Liste der Szenarioexits' 'NAK-286-rot-M-11-nach1.txt' 'rot' @() @(
    (Ersetzung $LZ @'
        $zusammen = ($ergebnisse | ForEach-Object { "{0}={1}" -f $_.Datei, $_.Exit }) -join ','
'@ @'
        $zusammen = ($ergebnisse | Where-Object { $_.Exit -ne 5 } | ForEach-Object { "{0}={1}" -f $_.Datei, $_.Exit }) -join ',' # ROTBEWEIS M-11 (Kopfzeile Exit 5)
'@))))

$eintraege.Add((Eintrag 'nach' 'M-11' '(Protokoll Render)' 'Renderschritt nicht mehr ueber Log ins Protokoll geschrieben' 'ps:kopfzeile_und_protokoll' '\[ROT\] M-11 kopfzeile_und_protokoll:.*Rohdatei: Renderschritt nicht unter ## Protokoll' 'NAK-286-rot-M-11-nach1.txt' 'rot' @() @(
    (Ersetzung $LZ @'
    Log ("Render: Exit {0}, Dauer {1} s, Datei {2}, SHA-256 {3}, Grund {4}" -f $status.exit, $status.dauer_s, $status.datei, $status.sha256, $status.grund)
'@ @'
    $null = ("Render: Exit {0}, Dauer {1} s, Datei {2}, SHA-256 {3}, Grund {4}" -f $status.exit, $status.dauer_s, $status.datei, $status.sha256, $status.grund) # ROTBEWEIS M-11 (Protokoll Render)
'@))))

$eintraege.Add((Eintrag 'nach' 'M-11' '(Protokoll Briefkasten)' 'Briefkastenschritt nicht mehr ueber Log ins Protokoll geschrieben' 'ps:kopfzeile_und_protokoll' '\[ROT\] M-11 kopfzeile_und_protokoll:.*Rohdatei: Briefkastenschritt nicht unter ## Protokoll' 'NAK-286-rot-M-11-nach1.txt' 'rot' @() @(
    (Ersetzung $LZ @'
    Log "Briefkasten: $($K.DiagnoseOrdner) bereit, $n Antwortdatei(en) frueherer Laeufe entfernt"
'@ @'
    $null = "Briefkasten: $($K.DiagnoseOrdner) bereit, $n Antwortdatei(en) frueherer Laeufe entfernt" # ROTBEWEIS M-11 (Protokoll Briefkasten)
'@))))

$eintraege.Add((Eintrag 'nach' 'M-11' '(Abschnitt Szenario)' 'Abschnittskopf ## Szenario nicht geschrieben' 'sz:szenario_abschnitte' '\[ROT\] M-11 szenario_abschnitte:.*Szenario ohne eigenen Abschnitt' 'NAK-286-rot-M-11-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
    kopf = f"## Szenario `{szenario.get('id', szenario_pfad.stem)}` — {szenario.get('titel', '')}\n"
'@ @'
    kopf = "\n"  # ROTBEWEIS M-11 (Abschnitt Szenario): ohne eigenen Abschnitt
'@))))

$eintraege.Add((Eintrag 'nach' 'M-11' '(Abschnitt Schritt)' 'Unterabschnitt ### Schritt einer lokalen Aktion nicht geschrieben' 'sz:szenario_abschnitte' '\[ROT\] M-11 szenario_abschnitte:.*Einzelheiten einer lokalen Aktion ohne eigenen Unterabschnitt' 'NAK-286-rot-M-11-nach1.txt' 'rot' @() @(
    (Ersetzung $SZ @'
                lauf.details += ["", f"### Schritt {nr} `{aktion}`", ""] + zeilen
'@ @'
                lauf.details += zeilen  # ROTBEWEIS M-11 (Abschnitt Schritt): ohne Unterabschnitt
'@))))

# ================================================================ Gegenprobe
# P-20: die Mutation der Etappe 4 (nur die Pruefung vor dem Render) gegen den neuen Selbsttest. Die redundante Schranke
# $rest haelt die Renderfreigabe; erwartet: rot an Meldung und render.json, keine Zeile mit dem Traeger.
$eintraege.Add((Eintrag 'gegenprobe' 'M-03' '(Render, Mutation der Etappe 4)' 'nur die Pruefung vor dem Render entfernt, Schranke $rest bleibt' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden:.*Renderprozess trotz fremdem FL gestartet' 'NAK-286-rot-M-03-nach1.txt' 'ohne Traeger' @() @($M03Render[0])))

# ================================================================ Lauf
function Fahre ([string] $test, [string] $ausgabe)
{
    $art, $rest = $test -split ':', 2
    $befehl = switch ($art)
    {
        'ps' { @('pwsh', '-NoProfile', '-File', $LZ, '-Selbsttest', '-Nur', $rest) }
        'sz' { @('py', '-3.13', $SZ, '--selbsttest', '--nur', $rest) }
        default { throw "unbekannte Testart $art" }
    }
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = (Get-Command $befehl[0] -CommandType Application | Select-Object -First 1).Source
    foreach ($a in @($befehl | Select-Object -Skip 1)) { $info.ArgumentList.Add([string]$a) }
    $info.WorkingDirectory = $wurzel
    $info.UseShellExecute = $false
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
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

# Mit `pwsh -File` kommt `-Nur M-21,M-03` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($eintraege | Where-Object { $_.Art -eq $Art -and ($Nur.Count -eq 0 -or $Nur -contains $_.Zeile -or $Nur -contains ($_.Zeile + ' ' + $_.Teil).Trim()) })
$zusammenfassung = New-Object System.Collections.Generic.List[string]
$nakamaHead = (& git -C $wurzel rev-parse HEAD)
$mcpHead = (& git -C $MCP rev-parse HEAD)
$artWort = @{ vorher = 'Rotlauf vor der Aenderung'; nach = 'Rotbeweis'; gegenprobe = 'Gegenprobe' }[$Art]

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
        $summeGruen = [string](@($gruenZeilen | Where-Object { $_ -match '^SELBSTTEST' }) | Select-Object -Last 1)
    }

    $gruende = @()
    if ($null -eq $rot -or $rot.Exit -eq 0) { $gruende += "Lauf rot Exit $(if ($rot) { $rot.Exit } else { 'fehlt' })" }
    if ($m.Erwartung -eq 'rot' -and $traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if ($m.Erwartung -eq 'ohne Traeger' -and $traeger.Count -ne 0) { $gruende += "Traegerzeilen $($traeger.Count) statt 0" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if ($mutiert -and $gruen.Exit -ne 0) { $gruende += "Lauf gruen Exit $($gruen.Exit)" }
    $gueltigWort = switch ($Art) { 'vorher' { if ($m.Erwartung -eq 'rot') { 'ROTLAUF GUELTIG' } else { 'NACHWEIS ERBRACHT' } } 'nach' { 'ROTBEWEIS GUELTIG' } 'gegenprobe' { 'NACHWEIS ERBRACHT' } }
    $ergebnis = if ($gruende.Count -eq 0) { $gueltigWort } else { $gueltigWort -replace 'GUELTIG|ERBRACHT', 'UNGUELTIG' }
    if ($gruende.Count -gt 0) { $ergebnis = "$ergebnis (" + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $m.Roh)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 4 Nacharbeit 1, $artWort $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Art: $artWort (§5.1, §40): $($m.Beschreibung)")
    $zeilen.Add("Nakama HEAD $nakamaHead (Arbeitsstand der Nacharbeit uncommittet); MCP-Repo HEAD $mcpHead")
    $zeilen.Add("Bein einzeln: $($rot.Befehl) (Obergrenze $ObergrenzeSekunden s)")
    $zeilen.Add("Traeger (Muster des Traegers): $($m.Traeger)")
    $zeilen.Add("Erwartung: $($m.Erwartung)")
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
