<#
NAK-286 Etappe 4 - Rotskript der Matrixzeilen des Laufzeit-Arms (Manifest docs/beweise/NAK-286.md, §5.1, §5.7).

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4.ps1                 alle Mutationen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4.ps1 -Nur M-10,M-65  nur diese Zeilen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe4.ps1 -NurPruefen     nur Traegertexte zaehlen

Je Mutation, in dieser Reihenfolge (§5.1; PowerShell und Python ohne den LastWriteTime-Schritt):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot4).
  2. Mutation genau an der Traegerzeile: jeder Alttext muss genau einmal vorkommen.
  3. Das Bein einzeln: A35 als einzelner Selbsttestfall (laufzeit.ps1 -Selbsttest -Nur, szenario.py
     oder nulltest.py --selbsttest --nur), MCP-pytest als einzelner Test, M-73 als git grep. Gueltig nur
     mit Exit ungleich 0 (bei git grep: ein Treffer) und der verlangten Zahl Zeilen, die den Traeger nennen.
  4. Bytegleiche Ruecknahme aus den gesicherten Bytes, SHA-256 gleich Schritt 1, derselbe Lauf gruen.
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-<nn>.txt je Zeile (NAK-297: NAK-286-rot-NAK-297.txt), je Mutation
ein angehaengter Abschnitt. Eine ungueltige Mutation bricht den Lauf nach der Ruecknahme ab.

Der Lauf startet kein FL, installiert nichts und faehrt keinen Laufzeitlauf: jedes Bein laeuft gegen Attrappen.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen, [string] $NurArt = '')

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$tmp = Join-Path $env:TEMP 'nak286-rot4'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
$env:PYTHONIOENCODING = 'utf-8'
$env:PYTHONDONTWRITEBYTECODE = '1'

$LZ = 'tools/fl/laufzeit.ps1'
$SZ = 'tools/fl/szenario.py'
$NT = 'tools/fl/nulltest.py'
$U40 = 'docs/gesundheit/szenarien/u40-aktivitaetsgate.json'
$SR = 'docs/gesundheit/szenarien/snapshot-runde01.json'
$INJ = 'eq-copilot/install/nakama-installer-v1.json'
$INP = 'eq-copilot/install/Install-Nakama.ps1'
$MCP = 'C:\Users\phili\Projekte\fl-studio-mcp'
$DEV = "$MCP\fl_controller\device_FLStudioMCP.py"
$MID = "$MCP\src\fl_studio_mcp\utils\midi_connection.py"
$FEN = "$MCP\src\fl_studio_mcp\utils\fenster.py"
$TPS = 'tests/test_port_selection.py'
$TCP = 'tests/test_controller_probe_handlers.py'

function Pfad ([string] $d) { if ([IO.Path]::IsPathRooted($d)) { $d } else { Join-Path $wurzel $d } }
# Schreiben mit Wiederholung: ein Virenscanner oder ein gerade endender Python-Prozess haelt eine frisch
# geschriebene Quelle kurz als abgebildeten Bereich offen ("Der Vorgang ist bei einer Datei mit einem
# geoeffneten Bereich ... nicht anwendbar", Rotskriptlauf 15.09.2026 bei M-70 (1)).
function Schreibe ([string] $pfad, [byte[]] $bytes)
{
    for ($versuch = 1; ; $versuch++)
    {
        try { [IO.File]::WriteAllBytes($pfad, $bytes); return }
        catch { if ($versuch -ge 100) { throw }; Start-Sleep -Milliseconds 200 }
    }
}
function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Mutation ([string] $zeile, [string] $teil, [string] $beschreibung, [string] $test, [string] $traeger, [string] $roh, [object[]] $ersetzungen, [int] $mindestens = 1)
{
    [pscustomobject]@{ Zeile = $zeile; Teil = $teil; Beschreibung = $beschreibung; Test = $test; Traeger = $traeger; Roh = $roh; Ersetzungen = $ersetzungen; Mindestens = $mindestens }
}

# Die Mutationen, je an der Zeile, die die Zusage traegt (Rotbeweisspalte der Matrix).
# Test: 'ps:<Fall>' laufzeit.ps1 -Selbsttest -Nur, 'sz:<Fall>' szenario.py --selbsttest --nur,
# 'nt:<Fall>' nulltest.py --selbsttest --nur, 'mcp:<Knoten>[,<Knoten>]' pytest im MCP-Repo, 'grep:<Text>' git grep.
$mutationen = New-Object System.Collections.Generic.List[object]

# ---------------------------------------------------------------- tools/fl/laufzeit.ps1
$mutationen.Add((Mutation 'M-01' '' 'Lohnt-Es gibt ohne Produktpfad Ja = $true zurueck' 'ps:lohnt_nicht_ohne_produktpfad' '\[ROT\] M-01 lohnt_nicht_ohne_produktpfad' 'NAK-286-rot-M-01.txt' @(
    (Ersetzung $LZ @'
    return @{ Ja = $false; Grund = "Diff $($K.Basis)..HEAD ($($dateien.Count) Datei(en)) beruehrt keinen Produktpfad" }
'@ @'
    return @{ Ja = $true; Grund = "Diff $($K.Basis)..HEAD ($($dateien.Count) Datei(en)) beruehrt keinen Produktpfad" } # ROTBEWEIS M-01
'@))))

$mutationen.Add((Mutation 'M-02' '' 'gescheiterter Diff gibt Ja = $false zurueck' 'ps:lohnt_vier_faelle' '\[ROT\] M-02 lohnt_vier_faelle.*\(b\)' 'NAK-286-rot-M-02.txt' @(
    (Ersetzung $LZ @'
    if ($r.Exit -ne 0) { return @{ Ja = $true; Grund = "Diff nicht bestimmbar
'@ @'
    if ($r.Exit -ne 0) { return @{ Ja = $false; Grund = "Diff nicht bestimmbar
'@))))

$mutationen.Add((Mutation 'M-03' '' 'Pruefung auf ein fremdes FL vor dem Render entfernt' 'ps:fremdes_projekt_nie_beenden' '\[ROT\] M-03 fremdes_projekt_nie_beenden.*Render' 'NAK-286-rot-M-03.txt' @(
    (Ersetzung $LZ @'
    $fremd = @(FL-Instanzen | Where-Object { Ist-Fremd $_ })
    if ($fremd.Count -gt 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen vor dem Render: '$($fremd[0].Titel)'" }
'@ @'
    # ROTBEWEIS M-03: Pruefung vor dem Render entfernt
'@))))

$mutationen.Add((Mutation 'M-04' '' 'Exitpruefung von --hashen entfernt' 'ps:hashen_scheitert_exit3' '\[ROT\] M-04 hashen_scheitert_exit3' 'NAK-286-rot-M-04.txt' @(
    (Ersetzung $LZ @'
    if ($h.Exit -ne 0) { return @{ Ok = $false; Ueberspringen = $false; Grund = "--hashen Exit $($h.Exit)" } }
'@ @'
    # ROTBEWEIS M-04: Exitpruefung von --hashen entfernt
'@))))

$mutationen.Add((Mutation 'M-05' '' 'Installationsbedingung prueft nur Running' 'ps:installation_verweigert_exit3' '\[ROT\] M-05 installation_verweigert_exit3.*Exit 1' 'NAK-286-rot-M-05.txt' @(
    (Ersetzung $LZ @'
    if ($r.Exit -ne 0 -or $r.Zustand -eq 'Running') { return
'@ @'
    if ($r.Zustand -eq 'Running') { return
'@))))

$mutationen.Add((Mutation 'M-06' '(2)' 'Wertung des pruefen-Exits entfernt' 'ps:pruefen_nicht_aktuell_exit3' '\[ROT\] M-06 pruefen_nicht_aktuell_exit3.*pruefen Exit 1' 'NAK-286-rot-M-06.txt' @(
    (Ersetzung $LZ @'
    if ($p.Exit -ne 0 -or $p.Zustand -eq 'Running' -or $stand.Count -eq 0 -or $nichtAktuell.Count -gt 0) {
'@ @'
    if ($p.Zustand -eq 'Running' -or $stand.Count -eq 0 -or $nichtAktuell.Count -gt 0) { # ROTBEWEIS M-06 (2)
'@))))

$mutationen.Add((Mutation 'M-07' '' 'FL-Start vor Stelle-LoopMidi-Sicher gezogen' 'ps:loopmidi_und_restprozesse' '\[ROT\] M-07 loopmidi_und_restprozesse.*Reihenfolge' 'NAK-286-rot-M-07.txt' @(
    (Ersetzung $LZ @'
            Stelle-LoopMidi-Sicher
            Beende-Restprozesse
            $laufend = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
'@ @'
            Beende-Restprozesse
            $laufend = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
'@),
    (Ersetzung $LZ @'
            $K.Ping = Warte-Ping
'@ @'
            Stelle-LoopMidi-Sicher # ROTBEWEIS M-07: loopMIDI erst nach dem FL-Start
            $K.Ping = Warte-Ping
'@))))

$mutationen.Add((Mutation 'M-08' '(1)' 'Fristschleife ohne throw' 'ps:ping_frist_exit3' '\[ROT\] M-08 ping_frist_exit3' 'NAK-286-rot-M-08.txt' @(
    (Ersetzung $LZ @'
    throw "kein Ping binnen $($K.PingFristSekunden) s: $letzter; $(Boot-Marke-Text)"
'@ @'
    return @{ Ok = $false; Fehler = $letzter } # ROTBEWEIS M-08 (1): Fristschleife ohne throw
'@))))

$mutationen.Add((Mutation 'M-08' '(2)' 'Schonfrist nach der Boot-Marke entfernt' 'ps:schonfrist_vor_trigger' '\[ROT\] M-08 schonfrist_vor_trigger' 'NAK-286-rot-M-08.txt' @(
    (Ersetzung $LZ @'
            & $script:U.Schlafe 8
'@ @'
            # ROTBEWEIS M-08 (2): Schonfrist entfernt
'@))))

$mutationen.Add((Mutation 'M-09' '' 'Hashvergleich invertiert' 'ps:controller_veraltet_neu_installieren' '\[ROT\] M-09 controller_veraltet_neu_installieren' 'NAK-286-rot-M-09.txt' @(
    (Ersetzung $LZ @'
    if ($hRepo -eq $hInstalliert) { return }
'@ @'
    if ($hRepo -ne $hInstalliert) { return } # ROTBEWEIS M-09: Hashvergleich invertiert
'@))))

$mutationen.Add((Mutation 'M-10' '(1)' 'nur Szenario-Exit 4 zaehlt als verfehlt' 'ps:szenario_exitcodes' '\[ROT\] M-10 szenario_exitcodes.*Szenario-Exit 2: Runner-Exit 0' 'NAK-286-rot-M-10.txt' @(
    (Ersetzung $LZ @'
        $verfehlt = @($ergebnisse | Where-Object { $_.Exit -ne 0 })
'@ @'
        $verfehlt = @($ergebnisse | Where-Object { $_.Exit -eq 4 }) # ROTBEWEIS M-10 (1)
'@))))

$mutationen.Add((Mutation 'M-10' '(2)' 'Abbruch der Folge auch bei Szenario-Exit 5' 'ps:kette_erreicht_folgeszenarien' '\[ROT\] M-10 kette_erreicht_folgeszenarien.*lief nicht' 'NAK-286-rot-M-10.txt' @(
    (Ersetzung $LZ @'
        if ($code -eq 3) { break }
'@ @'
        if ($code -eq 3 -or $code -eq 5) { break } # ROTBEWEIS M-10 (2)
'@))))

$mutationen.Add((Mutation 'M-10' '(3)' 'Szenario-Exit 5 nur als verfehlt gezaehlt' 'ps:kette_erreicht_folgeszenarien' '\[ROT\] M-10 kette_erreicht_folgeszenarien.*Gesamturteil 4 VERFEHLT statt 3' 'NAK-286-rot-M-10.txt' @(
    (Ersetzung $LZ @'
        if ($ergebnisse | Where-Object { $_.Exit -eq 3 -or $_.Exit -eq 5 }) { Ende 3 'VORAUSSETZUNG' $zusatz }
'@ @'
        if ($ergebnisse | Where-Object { $_.Exit -eq 3 }) { Ende 3 'VORAUSSETZUNG' $zusatz } # ROTBEWEIS M-10 (3)
'@))))

$mutationen.Add((Mutation 'M-11' '' 'Urteilswort GEMESSEN in OK umbenannt' 'ps:kopfzeile_erste_fassung' '\[ROT\] M-11 kopfzeile_erste_fassung' 'NAK-286-rot-M-11.txt' @(
    (Ersetzung $LZ @'
        Ende 0 'GEMESSEN' $zusatz
'@ @'
        Ende 0 'OK' $zusatz # ROTBEWEIS M-11
'@))))

$mutationen.Add((Mutation 'M-12' '' 'Hashvergleich am Ende entfernt' 'ps:projekt_unveraendert' '\[ROT\] M-12 projekt_unveraendert' 'NAK-286-rot-M-12.txt' @(
    (Ersetzung $LZ @'
    if ($veraendert) { $code = 4; $urteil = 'VERFEHLT'; $zusatz = ("$zusatz $veraendert").Trim() }
'@ @'
    # ROTBEWEIS M-12: Hashvergleich am Ende entfernt
'@))))

$mutationen.Add((Mutation 'M-13' '' '-Beenden beendet jede FL64-Instanz statt nur die Diagnose-Instanz' 'ps:volllauf_erzwingen_beenden' '\[ROT\] M-13 volllauf_erzwingen_beenden' 'NAK-286-rot-M-13.txt' @(
    (Ersetzung $LZ @'
        if ($K.Beenden) { Beende-Diagnose-FL }
'@ @'
        if ($K.Beenden) { foreach ($i in @(FL-Instanzen)) { & $script:U.BeendeProzess $i.Id } } # ROTBEWEIS M-13
'@))))

$mutationen.Add((Mutation 'M-55' '(1)' 'Entfernen der Anfrage in Ende gestrichen' 'ps:anfrage_lebenszyklus' '\[ROT\] M-55 anfrage_lebenszyklus.*anfrage\.json liegt' 'NAK-286-rot-M-55.txt' @(
    (Ersetzung $LZ @'
    try { Raeume-Anfrage-Ab } catch { Log "Anfrage abraeumen fehlgeschlagen: $($_.Exception.Message)" }
'@ @'
    # ROTBEWEIS M-55 (1): Entfernen der Anfrage in Ende gestrichen
'@))))

$mutationen.Add((Mutation 'M-55' '(2) Runner' 'Aufraeumen in umgekehrter Reihenfolge: Antworten vor der Anfrage' 'ps:anfrage_lebenszyklus' '\[ROT\] M-55 anfrage_lebenszyklus.*Aufraeumen: Anfrage' 'NAK-286-rot-M-55.txt' @(
    (Ersetzung $LZ @'
    $anfrage = Join-Path $K.DiagnoseOrdner 'anfrage.json'
    if (& $script:U.Existiert $anfrage) {
        & $script:U.Loesche $anfrage
        Log 'Briefkasten: liegende anfrage.json entfernt'
    }
    $n = 0
    foreach ($name in @(& $script:U.Namen $K.AntwortOrdner)) {
        if ($name -match $script:AntwortMuster -or $name -match $script:TempMuster) {
            & $script:U.Loesche (Join-Path $K.AntwortOrdner $name)
            $n++
        }
    }
'@ @'
    $n = 0
    foreach ($name in @(& $script:U.Namen $K.AntwortOrdner)) {
        if ($name -match $script:AntwortMuster -or $name -match $script:TempMuster) {
            & $script:U.Loesche (Join-Path $K.AntwortOrdner $name)
            $n++
        }
    }
    # ROTBEWEIS M-55 (2): Aufraeumen in umgekehrter Reihenfolge
    $anfrage = Join-Path $K.DiagnoseOrdner 'anfrage.json'
    if (& $script:U.Existiert $anfrage) {
        & $script:U.Loesche $anfrage
        Log 'Briefkasten: liegende anfrage.json entfernt'
    }
'@))))

$mutationen.Add((Mutation 'M-59' '' 'Schluessel frischer_start ignoriert' 'ps:frischer_start_schluessel' '\[ROT\] M-59 frischer_start_schluessel' 'NAK-286-rot-M-59.txt' @(
    (Ersetzung $LZ @'
        if ($kopf -and $kopf.frischer_start -eq $true) {
'@ @'
        if ($false -and $kopf -and $kopf.frischer_start -eq $true) { # ROTBEWEIS M-59
'@))))

$mutationen.Add((Mutation 'M-60' '(1) Runner' 'fehlende Renderdatei ohne Grund (fehlende Datei = uebersprungen)' 'ps:render_ablauf' '\[ROT\] M-60 render_ablauf.*ohne Datei' 'NAK-286-rot-M-60.txt' @(
    (Ersetzung $LZ @'
        else { $status.grund = "keine Datei: $wav fehlt nach Exit $($status.exit)" }
'@ @'
        else { $status.grund = $null } # ROTBEWEIS M-60 (1): fehlende Datei = uebersprungen
'@))))

$mutationen.Add((Mutation 'M-65' '(1)' 'Rueckweg auch bei KETTE oder VERSATZ' 'ps:rueckweg_nur_bei_abweichung' '\[ROT\] M-65 rueckweg_nur_bei_abweichung.*(KETTE|VERSATZ): Rueckweg gefahren' 'NAK-286-rot-M-65.txt' @(
    (Ersetzung $LZ @'
        if ($name -eq 'nulltest-host.json' -and (Nulltest-Urteil) -eq 'ABWEICHUNG') { $rueckweg = Fahre-Rueckweg }
'@ @'
        if ($name -eq 'nulltest-host.json' -and (Nulltest-Urteil) -in @('ABWEICHUNG', 'KETTE', 'VERSATZ')) { $rueckweg = Fahre-Rueckweg } # ROTBEWEIS M-65 (1)
'@))))

$mutationen.Add((Mutation 'M-65' '(2)' 'Rueckweg bei ABWEICHUNG entfaellt' 'ps:rueckweg_nur_bei_abweichung' '\[ROT\] M-65 rueckweg_nur_bei_abweichung.*ABWEICHUNG: kein Rueckweg' 'NAK-286-rot-M-65.txt' @(
    (Ersetzung $LZ @'
        if ($name -eq 'nulltest-host.json' -and (Nulltest-Urteil) -eq 'ABWEICHUNG') { $rueckweg = Fahre-Rueckweg }
'@ @'
        # ROTBEWEIS M-65 (2): Rueckweg bei ABWEICHUNG entfaellt
'@))))

$mutationen.Add((Mutation 'M-65' '(3)' 'Rueckweg vor dem Beenden des Diagnose-FL' 'ps:rueckweg_nur_bei_abweichung' '\[ROT\] M-65 rueckweg_nur_bei_abweichung.*FL-Prozess\(e\) leben waehrend des Rueckwegs' 'NAK-286-rot-M-65.txt' @(
    (Ersetzung $LZ @'
    Beende-Diagnose-FL
    Beende-Restprozesse
    $laufend = @(FL-Instanzen)
'@ @'
    $laufend = @() # ROTBEWEIS M-65 (3): Beenden erst nach dem Rueckwegaufruf
'@),
    (Ersetzung $LZ @'
    try { $r = Aufgabe 'rueckweg' } catch {
'@ @'
    try { $r = Aufgabe 'rueckweg'; Beende-Diagnose-FL; Beende-Restprozesse } catch {
'@))))

$mutationen.Add((Mutation 'M-65' '(4)' 'Kopfzeile ohne Grund: FL laeuft und NAK-41 ergeben denselben Text' 'ps:rueckweg_nur_bei_abweichung' '\[ROT\] M-65 rueckweg_nur_bei_abweichung.*FL laeuft: Kopfzeile' 'NAK-286-rot-M-65.txt' @(
    (Ersetzung $LZ @'
        $grund = 'Rueckweg verweigert (FL laeuft): ' + (($laufend | ForEach-Object { "PID $($_.Id) '$($_.Titel)'" }) -join ', ')
'@ @'
        $grund = 'Rueckweg verweigert' # ROTBEWEIS M-65 (4)
'@),
    (Ersetzung $LZ @'
    if ((@($r.Zeilen) -join "`n") -match 'Rueckweg verweigert \(NAK-41\)') { return "Rueckweg verweigert (NAK-41); pruefen Exit $($p.Exit)" }
'@ @'
    if ((@($r.Zeilen) -join "`n") -match 'Rueckweg verweigert \(NAK-41\)') { return 'Rueckweg verweigert' } # ROTBEWEIS M-65 (4)
'@))))

$mutationen.Add((Mutation 'M-65' '(5)' 'Folgeszenarien nach dem Rueckweg ohne Meldung uebersprungen' 'ps:rueckweg_nur_bei_abweichung' '\[ROT\] M-65 rueckweg_nur_bei_abweichung.*Rohdatei ohne Zeilen' 'NAK-286-rot-M-65.txt' @(
    (Ersetzung $LZ @'
            $entfallen += $name
            Log "Szenario ${name}: entfallen: Rueckweg nach ABWEICHUNG"
            Add-Content -LiteralPath $K.Roh -Value "`n## Szenario ``$name`` - entfallen: Rueckweg nach ABWEICHUNG`n" -Encoding utf8
'@ @'
            # ROTBEWEIS M-65 (5): ohne Meldung uebersprungen
'@))))

$mutationen.Add((Mutation 'NAK-297' '' 'Titel nur aus MainWindowTitle (erste Fassung)' 'ps:titel_ohne_haupttitel' '\[ROT\] NAK-297 titel_ohne_haupttitel' 'NAK-286-rot-NAK-297.txt' @(
    (Ersetzung $LZ @'
        if ($haupt.Count -gt 0 -and $haupt[0].Titel) { $titel = [string]$haupt[0].Titel }
'@ @'
        if ($false) { $titel = [string]$haupt[0].Titel } # ROTBEWEIS NAK-297: Titel nur aus MainWindowTitle
'@))))

# ---------------------------------------------------------------- tools/fl/szenario.py
$mutationen.Add((Mutation 'M-20' '(2)' 'fester Vorgabewert 4,0 ohne frist_s wiederhergestellt' 'sz:frist_ohne_schrittwert' '\[ROT\] M-20 frist_ohne_schrittwert' 'NAK-286-rot-M-20.txt' @(
    (Ersetzung $SZ @'
        frist = float(schritt["frist_s"]) if schritt.get("frist_s") is not None else None
'@ @'
        frist = float(schritt["frist_s"]) if schritt.get("frist_s") is not None else 4.0  # ROTBEWEIS M-20 (2)
'@))))

$mutationen.Add((Mutation 'M-78' '' 'Wiederholung in einer Schleife bis zum Erfolg' 'sz:timeout_genau_einmal' '\[ROT\] M-78 timeout_genau_einmal.*\(b\)' 'NAK-286-rot-M-78.txt' @(
    (Ersetzung $SZ @'
    if not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):
'@ @'
    while not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):  # ROTBEWEIS M-78
'@))))

$mutationen.Add((Mutation 'M-22' '' 'Import von fl_trigger in szenario.py' 'sz:kein_piano_roll_weg' '\[ROT\] M-22 kein_piano_roll_weg' 'NAK-286-rot-M-22.txt' @(
    (Ersetzung $SZ @'
def lokal_fenster(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
'@ @'
def lokal_fenster(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    from fl_studio_mcp.utils import fl_trigger  # noqa: F401 - ROTBEWEIS M-22
'@))))

$mutationen.Add((Mutation 'M-53' '(c1)' 'Groessenriegel im Runner entfernt: Name und Groesse, Lesegrenze, Groesse des Gelesenen' 'sz:antwortgroessen' '\[ROT\] M-53 antwortgroessen.*Groessenriegel: gen gewertet True' 'NAK-286-rot-M-53.txt' @(
    (Ersetzung $SZ @'
            if groesse < ANTWORT_MIN_BYTES or groesse > ANTWORT_MAX_BYTES:
                aus.verworfen.append((name, f"Groesse {groesse} Bytes ausserhalb [2 B, 16 MiB], nie gelesen"))
                continue
'@ @'
            # ROTBEWEIS M-53 (c1): Groessenriegel vor dem Oeffnen entfernt
'@),
    (Ersetzung $SZ @'
                roh = self.umg.lies(self.antworten / name, ANTWORT_MAX_BYTES)
'@ @'
                roh = self.umg.lies(self.antworten / name, 1 << 40)  # ROTBEWEIS M-53 (c1): ohne Lesegrenze
'@),
    (Ersetzung $SZ @'
        if len(roh) > ANTWORT_MAX_BYTES:
            return None, "beim Lesen groesser als 16 MiB"
'@ @'
        # ROTBEWEIS M-53 (c1): ohne Groessenpruefung des Gelesenen
'@))))

$mutationen.Add((Mutation 'M-53' '(c2)' 'Groessenpruefung vor dem Oeffnen entfernt' 'sz:antwortgroessen' '\[ROT\] M-53 antwortgroessen.*Leseoeffnungen vor der Groessenpruefung' 'NAK-286-rot-M-53.txt' @(
    (Ersetzung $SZ @'
            if groesse < ANTWORT_MIN_BYTES or groesse > ANTWORT_MAX_BYTES:
                aus.verworfen.append((name, f"Groesse {groesse} Bytes ausserhalb [2 B, 16 MiB], nie gelesen"))
                continue
'@ @'
            # ROTBEWEIS M-53 (c2): Groessenpruefung vor dem Oeffnen entfernt
'@))))

$mutationen.Add((Mutation 'M-55' '(2) Leser' 'Aufraeumen in umgekehrter Reihenfolge in szenario.py' 'sz:anfrage_schreiben_und_abraeumen' '\[ROT\] M-55 anfrage_schreiben_und_abraeumen.*Reihenfolge' 'NAK-286-rot-M-55.txt' @(
    (Ersetzung $SZ @'
        if self.umg.existiert(self.anfrage_pfad):
            self.umg.loesche(self.anfrage_pfad)
        for name, _groesse in self.umg.liste(self.antworten):
            if ANTWORT_NAME.match(name) or TEMP_NAME.match(name):
                self.umg.loesche(self.antworten / name)
'@ @'
        for name, _groesse in self.umg.liste(self.antworten):
            if ANTWORT_NAME.match(name) or TEMP_NAME.match(name):
                self.umg.loesche(self.antworten / name)
        if self.umg.existiert(self.anfrage_pfad):  # ROTBEWEIS M-55 (2): die Anfrage zuletzt
            self.umg.loesche(self.anfrage_pfad)
'@))))

$mutationen.Add((Mutation 'M-56' '' 'PID-Filter entfernt' 'sz:antworten_auswahl' '\[ROT\] M-56 antworten_auswahl' 'NAK-286-rot-M-56.txt' @(
    (Ersetzung $SZ @'
            if pid not in erwartet[rolle]:
                aus.fremd.append((name, f"fremd: PID {pid} nicht erwartet {sorted(erwartet[rolle])}"))
                continue
'@ @'
            # ROTBEWEIS M-56: PID-Filter entfernt
'@))))

$mutationen.Add((Mutation 'M-57' '' 'Broker immer erwartet' 'sz:ohne_brokerprozess_keine_erwartung' '\[ROT\] M-57 ohne_brokerprozess_keine_erwartung' 'NAK-286-rot-M-57.txt' @(
    (Ersetzung $SZ @'
            pids = lauf.umg.broker_pids()
            if pids:
'@ @'
            pids = lauf.umg.broker_pids()
            if True:  # ROTBEWEIS M-57: Broker immer erwartet
'@))))

$mutationen.Add((Mutation 'M-58' '' 'Vergleich min mit vertauschtem Operator' 'sz:erste_fassung_unveraendert' '\[ROT\] M-58 erste_fassung_unveraendert' 'NAK-286-rot-M-58.txt' @(
    (Ersetzung $SZ @'
                elif art == "min" and (not _ist_zahl(w) or w < soll):
'@ @'
                elif art == "min" and (not _ist_zahl(w) or w > soll):  # ROTBEWEIS M-58
'@))))

$mutationen.Add((Mutation 'M-66' '(1)' 'Delta_K fest 0' 'sz:kettenverschiebung_aus_nulltest' '\[ROT\] M-66 kettenverschiebung_aus_nulltest.*\(1\) KETTE' 'NAK-286-rot-M-66.txt' @(
    (Ersetzung $SZ @'
            return {"urteil": urteil, "v": v, "g": g, "delta_k_db": 20.0 * math.log10(abs(g))}
'@ @'
            return {"urteil": urteil, "v": v, "g": g, "delta_k_db": 0.0}  # ROTBEWEIS M-66 (1): Delta_K fest 0
'@))))

$mutationen.Add((Mutation 'M-66' '(2)' 'fehlendes Nulltesturteil als BITIDENTISCH gelesen' 'sz:kettenverschiebung_aus_nulltest' '\[ROT\] M-66 kettenverschiebung_aus_nulltest.*\(2\) ohne ergebnis\.json' 'NAK-286-rot-M-66.txt' @(
    (Ersetzung $SZ @'
        return {"fehlt": "ohne Nulltesturteil: ergebnis.json fehlt oder ist fremd"}
'@ @'
        return {"urteil": "BITIDENTISCH", "v": 0, "g": 1.0, "delta_k_db": 0.0}  # ROTBEWEIS M-66 (2)
'@))))

$mutationen.Add((Mutation 'M-67' '(1)' 'Umlaufzuordnung (zurueckfallendes Materialende) und Vorbedingung entfernt' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(1\) gewertet' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
        if vorige_e is not None and e < vorige_e:
            verlauf.append(f"`{anfrage.kennung[:8]}` E {e} < {vorige_e}: Anlaufdaten des naechsten Umlaufs, nie gewertet")
            break
'@ @'
        # ROTBEWEIS M-67 (1): zurueckfallendes Materialende nicht geprueft
'@),
    (Ersetzung $SZ @'
    if s.get("zustand") != "messbereit":
'@ @'
    if False and s.get("zustand") != "messbereit":  # ROTBEWEIS M-67 (1): Vorbedingung entfernt
'@))))

$mutationen.Add((Mutation 'M-67' '(2)' 'U_oben entfernt' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(2\) aktiv_sekunden 55' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
    elif aktiv_zellen > u_oben_zellen:
        maengel.append(f"Vorbedingung: aktiv_sekunden {aktiv} > U_oben {u_oben_zellen / 10:.1f} s (ueber den Wrap integriert)")
'@ @'
    # ROTBEWEIS M-67 (2): U_oben entfernt
'@))))

$mutationen.Add((Mutation 'M-67' '(3)' 'Mitte R_LUFS um 1,0 LU verschoben' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*Grundlauf' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
    mitte = r + (kette["delta_k_db"] if band.get("delta_k") else 0.0)
'@ @'
    mitte = r + (kette["delta_k_db"] if band.get("delta_k") else 0.0) + (1.0 if band.get("kurz") == "LUFS" else 0.0)  # ROTBEWEIS M-67 (3)
'@))))

$mutationen.Add((Mutation 'M-67' '(4)' 'Referenz an der Positionsklammer statt am schweren Anker' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*Grundlauf' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
        ende = k + n - v
'@ @'
        ende = (round(antwort.anfrage.p_vor * RATE / 1000) if art == "schwer" and antwort.anfrage and antwort.anfrage.p_vor is not None else k + n) - v  # ROTBEWEIS M-67 (4)
'@))))

$mutationen.Add((Mutation 'M-67' '(5)' 'U_unten auf dem Quellraster statt Minimum ueber alle Rasterversaetze' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(5\) phasenverschobene Attrappe' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
    return {"s0": s0, "n_l": n_l, "min_zellen": int(z.min()), "phi_min": int(z.argmin()),
'@ @'
    return {"s0": s0, "n_l": n_l, "min_zellen": int(z[(-s0) % ZELLE]), "phi_min": (-s0) % ZELLE,  # ROTBEWEIS M-67 (5): Quellraster
'@))))

$mutationen.Add((Mutation 'M-67' '(6)' 'Referenz als Praefix [0, p - v) mit Tabellenende am Loopende' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(6\) v = -75 600' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
        grenze_unten, grenze_oben = max(0, -v), min(n_loop - v, QUELLE_FRAMES)
'@ @'
        grenze_unten, grenze_oben = 0, min(n_loop, QUELLE_FRAMES)  # ROTBEWEIS M-67 (6): Tabellenende am Loopende
'@),
    (Ersetzung $SZ @'
        a[art] = [max(0, k - v), ende]
'@ @'
        a[art] = [0, ende]  # ROTBEWEIS M-67 (6): Praefix
'@))))

$mutationen.Add((Mutation 'M-67' '(7)' 'Zuordnungsregel entfernt' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(7\) beide Rollen' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
            if kurz in gleich_verfehlt:
                gruende.append(f"Referenz oder Toleranz: {kurz}")
            else:
                gruende.append(f"Messung: {', '.join(sorted(verfehlt))} ({kurz})")
'@ @'
            gruende.append(f"Messung: {', '.join(sorted(verfehlt))} ({kurz})")  # ROTBEWEIS M-67 (7)
'@))))

$mutationen.Add((Mutation 'M-67' '(8)' 'Ende ohne Wrap entfernt: eine stehende Position beendet die Folge nicht' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*\(8\) Position steht' 'NAK-286-rot-M-67.txt' @(
    (Ersetzung $SZ @'
                        if ms == ms_alt:
                            ende = "Transport steht"
'@ @'
                        pass  # ROTBEWEIS M-67 (8): stehende Position beendet die Folge nicht
'@))))

$mutationen.Add((Mutation 'M-68' '(1)' 'Offline-Resonanzmenge ohne ihren letzten Wert (919,3 Hz) gelesen' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-68|\[ROT\] M-67 snapshot_messpunkt_und_baender.*Grundlauf' 'NAK-286-rot-M-68.txt' @(
    (Ersetzung $SZ @'
        offline = [float(x) for x in referenz["resonanzen"]]
'@ @'
        offline = [float(x) for x in referenz["resonanzen"]][:-1]  # ROTBEWEIS M-68 (1): ohne 919,3 Hz
'@))))

$mutationen.Add((Mutation 'M-68' '(2)' 'Szenario mit einer Grenze fuer centroid_mag_hz' 'sz:f28_nur_hergeleitete_baender' '\[ROT\] M-68 f28_nur_hergeleitete_baender.*centroid_mag_hz' 'NAK-286-rot-M-68.txt' @(
    (Ersetzung $SR @'
    {"kurz": "low_frac", "feld": "snapshot.spektral.low_frac",
'@ @'
    {"kurz": "centroid", "feld": "snapshot.spektral.centroid_mag_hz", "anker": "schwer", "referenz": "centroid_mag", "toleranz": 50.0, "rollen": ["gen", "probeeq"], "rechnung": "F-28 ROTBEWEIS M-68 (2)"},
    {"kurz": "low_frac", "feld": "snapshot.spektral.low_frac",
'@))))

$mutationen.Add((Mutation 'M-68' '(3)' 'schwere Felder am leichten Anker n_L gelesen' 'sz:snapshot_messpunkt_und_baender' '\[ROT\] M-67 snapshot_messpunkt_und_baender.*Grundlauf' 'NAK-286-rot-M-68.txt' @(
    (Ersetzung $SZ @'
    for art, n in (("leicht", n_l), ("schwer", n_s)):
'@ @'
    for art, n in (("leicht", n_l), ("schwer", n_l)):  # ROTBEWEIS M-68 (3)
'@))))

$mutationen.Add((Mutation 'M-69' '' 'Regel waehlt das Maximum statt des Minimums' 'sz:stellen_aus_energieprofil' '\[ROT\] M-69 stellen_aus_energieprofil' 'NAK-286-rot-M-69.txt' @(
    (Ersetzung $SZ @'
    _mittel, takt, von, bis = min(fenster)
'@ @'
    _mittel, takt, von, bis = max(fenster)  # ROTBEWEIS M-69
'@))))

$mutationen.Add((Mutation 'M-70' '(1)' 'Zaehlung nach der Positionsklammer statt nach dem Materialausschnitt' 'sz:stelle_materialausschnitt' '\[ROT\] M-70 stelle_materialausschnitt.*\(1\) Rahmen aus der Zeit vor dem Start' 'NAK-286-rot-M-70.txt' @(
    (Ersetzung $SZ @'
        elif not (a_frames <= start and start + anzahl <= b_frames):
'@ @'
        elif not (anfrage.p_vor is not None and a_frames <= round(anfrage.p_vor * RATE / 1000) <= b_frames):  # ROTBEWEIS M-70 (1)
'@))))

$mutationen.Add((Mutation 'M-70' '(2)' 'Stelle ohne Versatz in den Host gelegt' 'sz:stelle_materialausschnitt' '\[ROT\] M-70 stelle_materialausschnitt.*\(2\) v = \+75 600' 'NAK-286-rot-M-70.txt' @(
    (Ersetzung $SZ @'
        a = round(float(stelle["von_s"]) * RATE) + v
        b = round(float(stelle["bis_s"]) * RATE) + v
'@ @'
        a = round(float(stelle["von_s"]) * RATE)  # ROTBEWEIS M-70 (2): ohne Versatz
        b = round(float(stelle["bis_s"]) * RATE)
'@))))

$mutationen.Add((Mutation 'M-70' '(3)' 'null als 0 gelesen (Zaehlung und Ausschnitt der Differenzen)' 'sz:stelle_materialausschnitt' "\[ROT\] M-70 stelle_materialausschnitt.*\(3\) projekt_sample_start null: Zaehlung 'gezaehlt'" 'NAK-286-rot-M-70.txt' @(
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

$mutationen.Add((Mutation 'M-70' '(4)' 'Ende der Anfragen entfernt' 'sz:stelle_materialausschnitt' '\[ROT\] M-70 stelle_materialausschnitt.*\(4\) Position steht' 'NAK-286-rot-M-70.txt' @(
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

$mutationen.Add((Mutation 'M-71' '(1)' 'roh schreibt null als 0' 'sz:roh_schreibt_null_als_null' '\[ROT\] M-71 roh_schreibt_null_als_null' 'NAK-286-rot-M-71.txt' @(
    (Ersetzung $SZ @'
    werte = " | ".join(_zelle(_json(antwort.wert(f"frame.{f}"))) for f in F23_FELDER)
'@ @'
    werte = " | ".join(_zelle(_json(0 if antwort.wert(f"frame.{f}") is None else antwort.wert(f"frame.{f}"))) for f in F23_FELDER)  # ROTBEWEIS M-71 (1)
'@))))

$mutationen.Add((Mutation 'M-71' '(2)' 'Delta aus den offenen Fenstern statt aus den Summen' 'sz:deltas_ueber_evidenzabschluesse' '\[ROT\] M-71 deltas_ueber_evidenzabschluesse.*Differenzen nicht aus den kumulativen Zaehlern' 'NAK-286-rot-M-71.txt' @(
    (Ersetzung $SZ @'
            "d_summe_fenster_aktiv": differenz("summe_fenster_aktiv"),
            "d_summe_fenster_gesamt": differenz("summe_fenster_gesamt"),
'@ @'
            "d_summe_fenster_aktiv": differenz("offen_fenster_aktiv"),  # ROTBEWEIS M-71 (2)
            "d_summe_fenster_gesamt": differenz("offen_fenster_gesamt"),
'@))))

$mutationen.Add((Mutation 'M-72' '(1)' 'Szenario mit abdeckung min 0,5' 'sz:u40_ohne_sollwert' '\[ROT\] M-72 u40_ohne_sollwert' 'NAK-286-rot-M-72.txt' @(
    (Ersetzung $U40 @'
  {"aktion": "lokal.stellen", "params": {"rollen": ["gen", "probeeq"], "stellen": [
'@ @'
  {"aktion": "lokal.stellen", "erwarte": {"frame.abdeckung": {"min": 0.5}}, "params": {"rollen": ["gen", "probeeq"], "stellen": [
'@))))

$mutationen.Add((Mutation 'M-72' '(2)' 'Plausibilitaet auf die offenen Fenster gelegt' 'sz:deltas_ueber_evidenzabschluesse' '\[ROT\] M-72 deltas_ueber_evidenzabschluesse|\[ROT\] M-71 deltas_ueber_evidenzabschluesse.*korrekter Bau als unplausibel' 'NAK-286-rot-M-72.txt' @(
    (Ersetzung $SZ @'
    aktiv, gesamt = delta["d_summe_fenster_aktiv"], delta["d_summe_fenster_gesamt"]
'@ @'
    aktiv, gesamt = (gezaehlt[-1].wert("frame.offen_fenster_aktiv") - gezaehlt[0].wert("frame.offen_fenster_aktiv"), gezaehlt[-1].wert("frame.offen_fenster_gesamt") - gezaehlt[0].wert("frame.offen_fenster_gesamt"))  # ROTBEWEIS M-72 (2)
'@))))

$mutationen.Add((Mutation 'M-81' '(3)' 'Fortlaufbedingung in szenario.py entfernt' 'sz:fortlaufbedingung_teilbloecke' '\[ROT\] M-81 fortlaufbedingung_teilbloecke' 'NAK-286-rot-M-81.txt' @(
    (Ersetzung $SZ @'
    if not (isinstance(fortlaufend, int) and isinstance(stillstand, int) and fortlaufend == n_l and stillstand == 0):
'@ @'
    if False:  # ROTBEWEIS M-81 (3): Fortlaufbedingung entfernt
'@))))

# ---------------------------------------------------------------- tools/fl/nulltest.py
$mutationen.Add((Mutation 'M-60' '(2) Nulltest' 'fehlende Renderdatei = uebersprungen: KEIN_RENDER mit Szenario-Exit 0' 'nt:render_status' '\[ROT\] M-60 render_status' 'NAK-286-rot-M-60.txt' @(
    (Ersetzung $NT @'
                  "KEIN_RENDER": EXIT_SZENARIO_VORAUSSETZUNG, "FORMATFEHLER": EXIT_SZENARIO_VORAUSSETZUNG}
'@ @'
                  "KEIN_RENDER": EXIT_OK, "FORMATFEHLER": EXIT_SZENARIO_VORAUSSETZUNG}  # ROTBEWEIS M-60 (2)
'@))))

$mutationen.Add((Mutation 'M-61' '' 'Formatpruefung entfernt: Bittiefe nicht geprueft' 'nt:format_ist_voraussetzung' '\[ROT\] M-61 format_ist_voraussetzung.*pcm16: Urteil ABWEICHUNG Exit 4' 'NAK-286-rot-M-61.txt' @(
    (Ersetzung $NT @'
    return (w.tag == PCM and w.bits == 24) or (w.tag == IEEE_FLOAT and w.bits == 32)
'@ @'
    return w.tag in (PCM, IEEE_FLOAT)  # ROTBEWEIS M-61: Bittiefe nicht geprueft
'@))))

$mutationen.Add((Mutation 'M-62' '(a)' 'Vergleich mit Toleranz 1e-6 statt bitgenau: das gekippte LSB faellt nicht auf' 'nt:identisch_null_abweichungen' '\[ROT\] M-62 identisch_null_abweichungen.*gekipptes LSB' 'NAK-286-rot-M-62.txt' @(
    (Ersetzung $NT @'
    ungleich = r != q
'@ @'
    ungleich = np.abs(r - q) > 1e-6  # ROTBEWEIS M-62 (a): Toleranz statt bitgenau
'@))))

$mutationen.Add((Mutation 'M-62' '(b)' 'Versatzsuche entfernt' 'nt:identisch_null_abweichungen' '\[ROT\] M-62 identisch_null_abweichungen.*ein Nullframe vor der Quelle' 'NAK-286-rot-M-62.txt' @(
    (Ersetzung $NT @'
    v = versatz_kandidat(render, quelle, n, suche)
'@ @'
    v = 0  # ROTBEWEIS M-62 (b): Versatzsuche entfernt
'@))))

$mutationen.Add((Mutation 'M-75' '(1)' 'Versatzurteil entfernt' 'nt:versatz_ist_voraussetzung' '\[ROT\] M-75 versatz_ist_voraussetzung.*v = 1: Urteil ABWEICHUNG Exit 4' 'NAK-286-rot-M-75.txt' @(
    (Ersetzung $NT @'
        ergebnis["urteil"] = "BITIDENTISCH" if v == 0 else "VERSATZ"
'@ @'
        ergebnis["urteil"] = "BITIDENTISCH" if v == 0 else "ABWEICHUNG"  # ROTBEWEIS M-75 (1)
'@))))

$mutationen.Add((Mutation 'M-75' '(2)' 'Suche auf +-4 410 Frames begrenzt' 'nt:versatz_ist_voraussetzung' '\[ROT\] M-75 versatz_ist_voraussetzung.*v = 75600: Urteil ABWEICHUNG' 'NAK-286-rot-M-75.txt' @(
    (Ersetzung $NT @'
    v = versatz_kandidat(render, quelle, n, suche)
'@ @'
    v = versatz_kandidat(render, quelle, n, 4410)  # ROTBEWEIS M-75 (2)
'@))))

$mutationen.Add((Mutation 'M-75' '(3)' 'Vorzeichen vertauscht: v als der Quellframe, an dem Renderframe 0 liegt' 'nt:versatz_ist_voraussetzung' '\[ROT\] M-75 versatz_ist_voraussetzung.*v = 75600: gemeldet v = -75600' 'NAK-286-rot-M-75.txt' @(
    (Ersetzung $NT @'
    v = versatz_kandidat(render, quelle, n, suche)
'@ @'
    v = -versatz_kandidat(render, quelle, n, suche)  # ROTBEWEIS M-75 (3)
'@))))

$mutationen.Add((Mutation 'M-63' '(1)' 'Klassifikation KETTE entfernt' 'nt:konstanter_faktor_ist_voraussetzung' '\[ROT\] M-63 konstanter_faktor_ist_voraussetzung.*Faktor 0,5: Urteil ABWEICHUNG Exit 4' 'NAK-286-rot-M-63.txt' @(
    (Ersetzung $NT @'
    if erklaert:
'@ @'
    if False:  # ROTBEWEIS M-63 (1): Klassifikation entfernt
'@))))

$mutationen.Add((Mutation 'M-63' '(2)' 'Restpruefung entfernt' 'nt:konstanter_faktor_ist_voraussetzung' '\[ROT\] M-63 konstanter_faktor_ist_voraussetzung.*mit Stoerung: Urteil KETTE Exit 5' 'NAK-286-rot-M-63.txt' @(
    (Ersetzung $NT @'
    erklaert = bool(np.all(rest <= toleranz))
'@ @'
    erklaert = True  # ROTBEWEIS M-63 (2): Restpruefung entfernt
'@))))

# ---------------------------------------------------------------- MCP-Repo (pytest einzeln)
$mutationen.Add((Mutation 'M-14' '' 'choose_port gibt am Ende den ersten Port zurueck' "mcp:${TPS}::test_no_fallback_to_first_port,${TPS}::test_env_override_that_matches_nothing_yields_none,${TPS}::test_connect_reports_missing_port_honestly" 'FAILED .*::(test_no_fallback_to_first_port|test_env_override_that_matches_nothing_yields_none|test_connect_reports_missing_port_honestly)' 'NAK-286-rot-M-14.txt' @(
    (Ersetzung $MID @'
    return candidates[0] if len(candidates) == 1 else None
'@ @'
    return candidates[0] if len(candidates) == 1 else (output_ports[0] if output_ports else None)  # ROTBEWEIS M-14
'@)) 3))

$mutationen.Add((Mutation 'M-15' '(2)' 'Mehrdeutigkeitspruefung entfernt: der erste Kandidat wird verbunden' "mcp:${TPS}::test_ambiguous_virtual_ports_are_not_connected" 'FAILED .*::test_ambiguous_virtual_ports_are_not_connected' 'NAK-286-rot-M-15.txt' @(
    (Ersetzung $MID @'
        candidates = port_candidates(output_ports)
        if len(candidates) > 1:
'@ @'
        candidates = port_candidates(output_ports)
        if False:  # ROTBEWEIS M-15 (2): Mehrdeutigkeitspruefung entfernt
'@),
    (Ersetzung $MID @'
    return candidates[0] if len(candidates) == 1 else None
'@ @'
    return candidates[0] if candidates else None  # ROTBEWEIS M-15 (2)
'@))))

$mutationen.Add((Mutation 'M-16' '' '_check_track in handle_mixer_get_peaks entfernt' "mcp:${TCP}::test_peaks_reject_out_of_range_track" 'FAILED .*::test_peaks_reject_out_of_range_track' 'NAK-286-rot-M-16.txt' @(
    (Ersetzung $DEV @'
    track = _check_track("track", params.get("track", 0), count)
    return {
        "track": track,
        "name": mixer.getTrackName(track),
        "peak_left": mixer.getTrackPeaks(track, PEAK_L),
'@ @'
    track = params.get("track", 0)  # ROTBEWEIS M-16: _check_track entfernt
    return {
        "track": track,
        "name": mixer.getTrackName(track),
        "peak_left": mixer.getTrackPeaks(track, PEAK_L),
'@))))

$mutationen.Add((Mutation 'M-17' '' 'MODE_SECONDS und MODE_ABS_TICKS vertauscht' "mcp:${TCP}::test_song_length_units_are_the_measured_ones" 'FAILED .*::test_song_length_units_are_the_measured_ones' 'NAK-286-rot-M-17.txt' @(
    (Ersetzung $DEV @'
MODE_MS, MODE_SECONDS, MODE_ABS_TICKS, MODE_BARS = 0, 1, 2, 3
'@ @'
MODE_MS, MODE_SECONDS, MODE_ABS_TICKS, MODE_BARS = 0, 2, 1, 3  # ROTBEWEIS M-17
'@))))

$mutationen.Add((Mutation 'M-18' '(2)' 'transport.setPosition gibt wieder getSongPosHint() zurueck' "mcp:${TCP}::test_set_position_makes_no_position_claim" 'FAILED .*::test_set_position_makes_no_position_claim' 'NAK-286-rot-M-18.txt' @(
    (Ersetzung $DEV @'
    return {"requested_position": position, "mode": mode}
'@ @'
    return {"position": transport.getSongPosHint()}  # ROTBEWEIS M-18 (2)
'@))))

$mutationen.Add((Mutation 'M-18' '(3)' 'transport.getPosition liest getSongPosHint() statt getSongPos' "mcp:${TCP}::test_get_position_reports_ms_and_seconds" 'FAILED .*::test_get_position_reports_ms_and_seconds' 'NAK-286-rot-M-18.txt' @(
    (Ersetzung $DEV @'
        "ms": transport.getSongPos(MODE_MS),
'@ @'
        "ms": transport.getSongPosHint(),  # ROTBEWEIS M-18 (3)
'@))))

$mutationen.Add((Mutation 'M-19' '(2)' 'volume_db aus dem normierten Wert gerechnet statt ueber den dB-Modus' "mcp:${TCP}::test_channel_info_reports_volume_db" 'FAILED .*::test_channel_info_reports_volume_db' 'NAK-286-rot-M-19.txt' @(
    (Ersetzung $DEV @'
        "volume_db": channels.getChannelVolume(index, True, use_global),
'@ @'
        "volume_db": round(40.0 * __import__("math").log10(max(channels.getChannelVolume(index, False, use_global), 1e-9)), 2),  # ROTBEWEIS M-19 (2)
'@))))

$mutationen.Add((Mutation 'M-20' '(3)' 'MCP: Zweig if timeout is None entfernt' "mcp:${TPS}::test_send_command_resolves_none_from_env" 'FAILED .*::test_send_command_resolves_none_from_env' 'NAK-286-rot-M-20.txt' @(
    (Ersetzung $MID @'
        if timeout is None:
            timeout = default_timeout()
'@ @'
        # ROTBEWEIS M-20 (3): Zweig if timeout is None entfernt
'@))))

$mutationen.Add((Mutation 'M-21' '' '_focus_fl_studio_windows (fl_trigger.py:91-148) vor der Erfassung gerufen' 'mcp:tests/test_fenster.py::test_capture_never_touches_foreground' 'FAILED .*::test_capture_never_touches_foreground' 'NAK-286-rot-M-21.txt' @(
    (Ersetzung $FEN @'
    window = windows[0]
    result = capture(window["hwnd"], Path(directory) / f"{name}.png")
'@ @'
    window = windows[0]
    from fl_studio_mcp.utils.fl_trigger import FLStudioTrigger  # ROTBEWEIS M-21
    FLStudioTrigger()._focus_fl_studio_windows()
    result = capture(window["hwnd"], Path(directory) / f"{name}.png")
'@))))

# ---------------------------------------------------------------- Nacharbeit aus dem Laufzeitlauf 1 (15.09.2026)
$mutationen.Add((Mutation 'M-57' '(2) tasklist' 'tasklist wieder mit text=True und ohne Rueckfall fuer stdout None gelesen' 'sz:tasklist_oem_ohne_treffer' '\[ROT\] M-57 tasklist_oem_ohne_treffer' 'NAK-286-rot-M-57.txt' @(
    (Ersetzung $SZ @'
                               capture_output=True, timeout=15)
        except (OSError, subprocess.SubprocessError):
            return []
        text = (r.stdout or b"").decode("oem" if os.name == "nt" else "utf-8", errors="replace")
'@ @'
                               capture_output=True, text=True, timeout=15)  # ROTBEWEIS M-57 (2)
        except (OSError, subprocess.SubprocessError):
            return []
        text = r.stdout
'@))))

$mutationen.Add((Mutation 'M-68' '(4)' 'low_frac-Referenz ueber den Mid-Mix statt ueber die Kanalenergie' 'sz:low_frac_in_snapshotdefinition' '\[ROT\] M-68 low_frac_in_snapshotdefinition' 'NAK-286-rot-M-68.txt' @(
    (Ersetzung $SZ @'
        summe += 0.5 * (np.abs(np.fft.rfft(seg[:, 0] * fenster)) ** 2 + np.abs(np.fft.rfft(seg[:, 1] * fenster)) ** 2)
'@ @'
        summe += np.abs(np.fft.rfft(0.5 * (seg[:, 0] + seg[:, 1]) * fenster)) ** 2  # ROTBEWEIS M-68 (4): Mid-Mix
'@))))

$mutationen.Add((Mutation 'M-70' '(5)' 'Zeitplan ohne Bewertung: der erste Kandidat wird genommen' 'sz:stelle_zeitplan' '\[ROT\] M-70 stelle_zeitplan' 'NAK-286-rot-M-70.txt' @(
    (Ersetzung $SZ @'
            if bester is None or (tragend, marge) > (bester[3], bester[2]):
'@ @'
            if bester is None:  # ROTBEWEIS M-70 (5): ohne Bewertung
'@))))

$mutationen.Add((Mutation 'M-70' '(6)' 'getrennte Durchlaeufe je Rolle entfernt: immer eine gemeinsame Anfragefolge' 'sz:stelle_zwei_durchlaeufe' '\[ROT\] M-70 stelle_zwei_durchlaeufe' 'NAK-286-rot-M-70.txt' @(
    (Ersetzung $SZ @'
    getrennt = len(rollen) > 1 and gemeinsam is not None and gemeinsam[3] < gemeinsam[4]
'@ @'
    getrennt = False  # ROTBEWEIS M-70 (6): immer gemeinsam
'@))))

$mutationen.Add((Mutation 'M-70' '(7)' 'erste Anfrage auch waehrend transport.start blockiert zugelassen' 'sz:stelle_zeitplan' '\[ROT\] M-70 stelle_zeitplan.*waehrend transport.start blockiert' 'NAK-286-rot-M-70.txt' @(
    (Ersetzung $SZ @'
            if t1 < jetzt + 0.1 or start - 0.02 <= t1 < start + 2.0 * befehl_s + 0.05:
'@ @'
            if t1 < jetzt + 0.1:  # ROTBEWEIS M-70 (7): Startfenster nicht ausgeschlossen
'@))))

$mutationen.Add((Mutation 'M-21' '(2)' 'Prototypentabelle wieder als dict mit Funktionszeigern als Schluessel' 'mcp:tests/test_fenster.py::test_declare_accepts_real_win32_function_pointers' 'FAILED .*::test_declare_accepts_real_win32_function_pointers' 'NAK-286-rot-M-21.txt' @(
    (Ersetzung $FEN @'
    prototypes = (
        (user32.EnumWindows, [ENUM_WINDOWS_PROC, wintypes.LPARAM], wintypes.BOOL),
        (user32.GetWindowThreadProcessId, [hwnd, ctypes.POINTER(wintypes.DWORD)], wintypes.DWORD),
        (user32.GetClassNameW, [hwnd, wintypes.LPWSTR, ctypes.c_int], ctypes.c_int),
        (user32.GetWindowTextW, [hwnd, wintypes.LPWSTR, ctypes.c_int], ctypes.c_int),
        (user32.IsWindowVisible, [hwnd], wintypes.BOOL),
        (user32.IsIconic, [hwnd], wintypes.BOOL),
        (user32.GetWindowRect, [hwnd, ctypes.POINTER(RECT)], wintypes.BOOL),
        (user32.GetDC, [hwnd], hdc),
        (user32.ReleaseDC, [hwnd, hdc], ctypes.c_int),
        (user32.PrintWindow, [hwnd, hdc, wintypes.UINT], wintypes.BOOL),
        (gdi32.CreateCompatibleDC, [hdc], hdc),
        (gdi32.CreateCompatibleBitmap, [hdc, ctypes.c_int, ctypes.c_int], hbitmap),
        (gdi32.SelectObject, [hdc, hgdiobj], hgdiobj),
        (gdi32.GetDIBits,
         [hdc, hbitmap, wintypes.UINT, wintypes.UINT, ctypes.c_void_p,
          ctypes.POINTER(BITMAPINFO), wintypes.UINT],
         ctypes.c_int),
        (gdi32.DeleteObject, [hgdiobj], wintypes.BOOL),
        (gdi32.DeleteDC, [hdc], wintypes.BOOL),
    )
    for function, argtypes, restype in prototypes:
'@ @'
    prototypes = {  # ROTBEWEIS M-21 (2): Funktionszeiger als dict-Schluessel
        user32.EnumWindows: ([ENUM_WINDOWS_PROC, wintypes.LPARAM], wintypes.BOOL),
        user32.GetWindowThreadProcessId: ([hwnd, ctypes.POINTER(wintypes.DWORD)], wintypes.DWORD),
        user32.GetClassNameW: ([hwnd, wintypes.LPWSTR, ctypes.c_int], ctypes.c_int),
        user32.GetWindowTextW: ([hwnd, wintypes.LPWSTR, ctypes.c_int], ctypes.c_int),
        user32.IsWindowVisible: ([hwnd], wintypes.BOOL),
        user32.IsIconic: ([hwnd], wintypes.BOOL),
        user32.GetWindowRect: ([hwnd, ctypes.POINTER(RECT)], wintypes.BOOL),
        user32.GetDC: ([hwnd], hdc),
        user32.ReleaseDC: ([hwnd, hdc], ctypes.c_int),
        user32.PrintWindow: ([hwnd, hdc, wintypes.UINT], wintypes.BOOL),
        gdi32.CreateCompatibleDC: ([hdc], hdc),
        gdi32.CreateCompatibleBitmap: ([hdc, ctypes.c_int, ctypes.c_int], hbitmap),
        gdi32.SelectObject: ([hdc, hgdiobj], hgdiobj),
        gdi32.GetDIBits: (
            [hdc, hbitmap, wintypes.UINT, wintypes.UINT, ctypes.c_void_p,
             ctypes.POINTER(BITMAPINFO), wintypes.UINT],
            ctypes.c_int,
        ),
        gdi32.DeleteObject: ([hgdiobj], wintypes.BOOL),
        gdi32.DeleteDC: ([hdc], wintypes.BOOL),
    }
    for function, (argtypes, restype) in prototypes.items():
'@))))

# ---------------------------------------------------------------- Installertexte (git grep)
$mutationen.Add((Mutation 'M-73' '' 'Texte der ersten Fassung zurueckgespielt (.NOTES und nicht_teil_dieses_manifests Punkt 4)' 'grep:von keiner Automatik' 'von keiner Automatik' 'NAK-286-rot-M-73.txt' @(
    (Ersetzung $INJ @'
    "Die Installation selbst: seit NAK-285 fahren die erhoehten Aufgaben \\Nakama\\installieren, \\Nakama\\pruefen und \\Nakama\\rueckweg das Installerskript mit diesem Manifest automatisch, als derselbe Benutzer mit hoechsten Rechten; der Laufzeit-Arm (tools/fl/laufzeit.ps1) startet sie ohne Klick. Von Hand bleibt es ein Aufruf mit Rechteerhoehung (NAK-32)."
'@ @'
    "Die Installation selbst: sie bleibt ein Klick des Users mit Rechteerhoehung (NAK-32). Dieses Manifest wird von keiner Automatik ausgefuehrt."
'@),
    (Ersetzung $INP @'
    Braucht Rechteerhoehung (Program Files). Automatischer Weg seit NAK-285:
    die erhoehten Aufgaben \Nakama\installieren, \Nakama\pruefen und
    \Nakama\rueckweg fahren dieses Skript als derselbe Benutzer mit hoechsten
    Rechten (einmal registriert ueber
    tools/dirigent/install-aufgaben-registrieren.ps1); der Laufzeit-Arm
    tools/fl/laufzeit.ps1 startet sie ohne Klick. Von Hand bleibt es ein
    Aufruf mit Rechteerhoehung (NAK-32).
'@ @'
    Braucht Rechteerhoehung (Program Files). Die Installation bleibt ein
    Klick des Users - dieses Skript wird von keiner Automatik gefahren
    (NAK-32).
'@)) 2))

function Fahre ([string] $test, [string] $ausgabe)
{
    $art, $rest = $test -split ':', 2
    $befehl = switch ($art)
    {
        'ps'   { @('pwsh', '-NoProfile', '-File', $LZ, '-Selbsttest', '-Nur', $rest) }
        'sz'   { @('py', '-3.13', $SZ, '--selbsttest', '--nur', $rest) }
        'nt'   { @('py', '-3.13', $NT, '--selbsttest', '--nur', $rest) }
        'mcp'  { @('uv', 'run', '--directory', $MCP, 'pytest') + @($rest -split ',') + @('-p', 'no:cacheprovider', '--color=no') }
        'grep' { @('git', 'grep', '-n', $rest, '--', 'eq-copilot/install') }
        default { throw "unbekannte Testart $art" }
    }
    $argumente = @($befehl | Select-Object -Skip 1)
    & $befehl[0] @argumente *> $ausgabe
    $exit = $LASTEXITCODE
    # git grep: Exit 0 = Treffer (die Zusage faellt, rot), 1 = kein Treffer (gruen).
    if ($art -eq 'grep') { $exit = if ($exit -eq 0) { 1 } elseif ($exit -eq 1) { 0 } else { $exit } }
    return [pscustomobject]@{ Exit = $exit; Befehl = ($befehl -join ' ') }
}

# Mit `pwsh -File` kommt `-Nur M-10,M-65` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($mutationen | Where-Object { $Nur.Count -eq 0 -or $Nur -contains $_.Zeile -or $Nur -contains ($_.Zeile + ' ' + $_.Teil).Trim() })
# -NurArt sz faehrt nur die Mutationen einer Testart (hier: nach einer Aenderung an szenario.py alle sz-Beine erneut).
if ($NurArt) { $auswahl = @($auswahl | Where-Object { $_.Test -like "${NurArt}:*" }) }
$zusammenfassung = New-Object System.Collections.Generic.List[string]
$nakamaHead = (& git -C $wurzel rev-parse HEAD)
$mcpHead = (& git -C $MCP rev-parse HEAD)

foreach ($m in $auswahl)
{
    $name = ($m.Zeile + ' ' + $m.Teil).Trim()
    $dateien = @($m.Ersetzungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $orig = @{}; $shaVor = @{}; $texte = @{}
    foreach ($d in $dateien)
    {
        $orig[$d] = [IO.File]::ReadAllBytes((Pfad $d))
        $shaVor[$d] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Pfad $d)).Hash
        [IO.File]::WriteAllBytes((Join-Path $tmp ((Split-Path $d -Leaf) + '.orig')), $orig[$d])
        $texte[$d] = $utf8.GetString($orig[$d])
    }
    $diff = New-Object System.Collections.Generic.List[string]
    foreach ($e in $m.Ersetzungen)
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
    if ($NurPruefen) { Write-Output "PRUEFUNG OK $name"; continue }

    $shaMut = @{}
    $rot = $null; $rotZeilen = @(); $traeger = @()
    $ausRot = Join-Path $tmp 'bein-rot.txt'
    if (Test-Path $ausRot) { [IO.File]::Delete($ausRot) }
    try
    {
        foreach ($d in $dateien)
        {
            Schreibe (Pfad $d) $utf8.GetBytes($texte[$d])
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
        foreach ($d in $dateien) { Schreibe (Pfad $d) $orig[$d] }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Pfad $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $ausGruen = Join-Path $tmp 'bein-gruen.txt'
    if (Test-Path $ausGruen) { [IO.File]::Delete($ausGruen) }
    $gruen = Fahre $m.Test $ausGruen
    $gruenZeilen = if (Test-Path $ausGruen) { @(Get-Content $ausGruen -Encoding utf8) } else { @() }
    $summeGruen = (@($gruenZeilen | Where-Object { $_ -match '^SELBSTTEST|passed|failed' }) | Select-Object -Last 1)
    if ($null -eq $summeGruen) { $summeGruen = if ($m.Test -like 'grep:*') { 'git grep ohne Treffer' } else { '' } }

    $gruende = @()
    if ($null -eq $rot -or $rot.Exit -eq 0) { $gruende += "Lauf rot Exit $(if ($rot) { $rot.Exit } else { 'fehlt' })" }
    if ($traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if ($gruen.Exit -ne 0) { $gruende += "Lauf gruen Exit $($gruen.Exit)" }
    $ergebnis = if ($gruende.Count -eq 0) { 'ROTBEWEIS GUELTIG' } else { 'ROTBEWEIS UNGUELTIG (' + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $m.Roh)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 4, Rotbeweis $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Art: Rotbeweis nach dem Bau (§5.1): Mutation am Traeger, Bein einzeln, bytegleiche Ruecknahme, gruen")
    $zeilen.Add("Nakama HEAD $nakamaHead (Arbeitsstand der Etappe uncommittet); MCP-Repo HEAD $mcpHead")
    $zeilen.Add("Mutation: $($m.Beschreibung)")
    $zeilen.Add("Bein einzeln: $($rot.Befehl)")
    $zeilen.Add("Traeger (Muster der Meldung): $($m.Traeger)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })")
    }
    $zeilen.Add("rot: Exit $($rot.Exit), Traegerzeilen $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add('--- Rohausgabe, mutierter Stand ---')
    foreach ($z in $rotZeilen) { $zeilen.Add($z) }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    $zeilen.Add("gruen: Exit $($gruen.Exit), $summeGruen")
    $zeilen.Add("Ergebnis: $ergebnis")
    $zeilen.Add('')
    # Anhaengen, nie ueberschreiben: ein frueherer Lauf derselben Zeile bleibt als Verlauf stehen.
    [IO.File]::AppendAllText($roh, (($zeilen -join "`n") + "`n"), $utf8)

    $shaText = ($dateien | ForEach-Object { "$(Split-Path $_ -Leaf) $($shaVor[$_].Substring(0,12))=$($shaNach[$_].Substring(0,12))" }) -join ', '
    $zeile = "$name | rot Exit $($rot.Exit), Traeger $($traeger.Count) | gruen Exit $($gruen.Exit) | $shaText | $ergebnis"
    $zusammenfassung.Add($zeile)
    Write-Output $zeile
    if ($gruende.Count -gt 0) { break }
}

[IO.File]::WriteAllText((Join-Path $tmp 'zusammenfassung.txt'), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE: $($zusammenfassung.Count) Mutationen, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
