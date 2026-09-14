<#
NAK-286 Etappe 3 - Rotskript der Matrixzeilen des Brokers (Manifest docs/beweise/NAK-286.md, §5.1, §5.6).

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3.ps1                 alle Mutationen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3.ps1 -Nur M-41,M-45  nur diese Zeilen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe3.ps1 -NurPruefen     nur Traegertexte zaehlen

Je Mutation, in dieser Reihenfolge (§5.1; Rust ohne den LastWriteTime-Schritt, Punkt 3):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot3).
  2. Mutation genau an der Traegerzeile: jeder Alttext muss genau einmal vorkommen.
  3. Bein A4 als Einzeltest (cargo test ... -- <Test> --exact): gueltig nur mit Exit ungleich 0
     und einer Zeile, die den Traeger der Zusage nennt.
  4. Bytegleiche Ruecknahme aus den gesicherten Bytes, SHA-256 gleich Schritt 1, derselbe
     Einzeltest gruen (Exit 0).
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-<nn>.txt (Zeilen M-41 bis M-47, M-76) und
docs/beweise/roh/NAK-286-rot-M-<nn>-rust.txt (Rust-Haelften M-48, M-49, M-51 bis M-53), je Mutation
ein angehaengter Abschnitt. Eine ungueltige Mutation bricht den Lauf nach der Ruecknahme ab.

M-46 mutiert voruebergehend broker/src/probe.rs (kein Ticketpfad): der mutierte probe_lauf startet den
Briefkasten mit den echten Fassaden. Das Skript verweigert diese Mutation, solange unter
%LOCALAPPDATA%\evenacadia\nakama\diagnose\ eine anfrage.json liegt.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$tmp = Join-Path $env:TEMP 'nak286-rot3'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)

$BK = 'broker/src/briefkasten.rs'
$LL = 'broker/src/lebenslauf.rs'
$PR = 'broker/src/probe.rs'

function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Mutation ([string] $zeile, [string] $teil, [string] $beschreibung, [string] $test, [string] $traeger, [string] $roh, [object[]] $ersetzungen)
{
    [pscustomobject]@{ Zeile = $zeile; Teil = $teil; Beschreibung = $beschreibung; Test = $test; Traeger = $traeger; Roh = $roh; Ersetzungen = $ersetzungen }
}

# Die Mutationen, je an der Zeile, die die Zusage traegt (Rotbeweisspalte der Matrix).
# Test: 'lib:<Name>' fuer die Unittests in broker/src/briefkasten.rs, 'int:<Name>' fuer broker/tests/briefkasten.rs.
$mutationen = New-Object System.Collections.Generic.List[object]

$mutationen.Add((Mutation 'M-41' '' 'Existenzpruefung entfernt, der Takt oeffnet anfrage.json direkt' 'lib:ohne_anfrage_nur_existenzpruefung' 'M-41 ohne_anfrage_nur_existenzpruefung taktfenster' 'NAK-286-rot-M-41.txt' @(
    (Ersetzung $BK '        let attribute = self.fassaden.dateisystem.attribute(&self.anfrage);' @'
        // ROTBEWEIS M-41: Existenzpruefung entfernt, der Takt oeffnet anfrage.json direkt
        let mut vorab = [0u8; ANFRAGE_GRENZE_BYTES + 1];
        let attribute = match self.fassaden.dateisystem.lies(&self.anfrage, &mut vorab) {
            Some(gelesen) => DateiAttribute { existiert: true, groesse: gelesen as u64, ..DateiAttribute::default() },
            None => DateiAttribute::default(),
        };
'@))))

$mutationen.Add((Mutation 'M-76' '(1)' 'Grund ueber fehler_merken ins Register eingetragen' 'lib:startfehler_ohne_registerzeile' 'M-76 startfehler_ohne_registerzeile durchlauf 1 \(LOCALAPPDATA fehlt\): Registerfehlerliste' 'NAK-286-rot-M-76.txt' @(
    (Ersetzung $BK '        griff.grund = Startgrund::KeineWurzel;' @'
        sperre(&register).fehler_merken("Briefkasten: LOCALAPPDATA ist nicht gesetzt".to_string()); // ROTBEWEIS M-76 (1)
        griff.grund = Startgrund::KeineWurzel;
'@))))

$mutationen.Add((Mutation 'M-76' '(2)' 'Ebenenpruefung an der Fassade vorbei (std::fs::symlink_metadata direkt)' 'lib:startfehler_ohne_registerzeile' 'M-76 startfehler_ohne_registerzeile durchlauf 2 \(evenacadia reparse\) startfenster' 'NAK-286-rot-M-76.txt' @(
    (Ersetzung $BK '        let vorgefunden = fassaden.dateisystem.attribute(ebene);' @'
        // ROTBEWEIS M-76 (2): Ebenenpruefung an der Fassade vorbei (std::fs::symlink_metadata direkt)
        let vorgefunden = std::fs::symlink_metadata(&ebene.pfad).map_or(DateiAttribute::default(), |meta| {
            use std::os::windows::fs::MetadataExt;
            let reparse = meta.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0;
            DateiAttribute { existiert: true, verzeichnis: meta.is_dir(), reparse, ..DateiAttribute::default() }
        });
'@))))

$mutationen.Add((Mutation 'M-42' '' 'aggregat als leeres Objekt im Umschlag' 'lib:antwort_feldmenge_wie_schema' 'M-42 antwort_feldmenge_wie_schema: \[' 'NAK-286-rot-M-42.txt' @(
    (Ersetzung $BK @'
        json!({
            "format": ANTWORT_FORMAT,
'@ @'
        let aggregat = if aggregat.is_null() { aggregat } else { json!({}) }; // ROTBEWEIS M-42: aggregat als leeres Objekt
        json!({
            "format": ANTWORT_FORMAT,
'@))))

$mutationen.Add((Mutation 'M-43' '(1)' 'Ringabfrage entfernt' 'lib:kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(M-25\) gleiche kennung keine zweite antwort' 'NAK-286-rot-M-43.txt' @(
    (Ersetzung $BK '        if zustand.ring_enthaelt(&kennung) {' '        if false && zustand.ring_enthaelt(&kennung) { // ROTBEWEIS M-43 (1): Ringabfrage entfernt'))))

$mutationen.Add((Mutation 'M-43' '(2)' 'Umbenennen mit Ersetzen (MOVEFILE_REPLACE_EXISTING)' 'lib:kennungsregeln_wie_plugin' 'M-43 kennungsregeln_wie_plugin \(M-33 \(d\)\) keine datei wird je ersetzt' 'NAK-286-rot-M-43.txt' @(
    (Ersetzung $BK '        unsafe { MoveFileExW(von.breit.as_ptr(), nach.breit.as_ptr(), MOVEFILE_WRITE_THROUGH) != 0 }' '        unsafe { MoveFileExW(von.breit.as_ptr(), nach.breit.as_ptr(), MOVEFILE_WRITE_THROUGH | windows_sys::Win32::Storage::FileSystem::MOVEFILE_REPLACE_EXISTING) != 0 } // ROTBEWEIS M-43 (2)'))))

$mutationen.Add((Mutation 'M-44' '' 'Sperrgrund nicht gefragt' 'lib:gesperrtes_aggregat_antwort_mit_grund' 'M-44 gesperrtes_aggregat_antwort_mit_grund: Aggregat im Umschlag trotz Sperre' 'NAK-286-rot-M-44.txt' @(
    (Ersetzung $BK '        let aggregat = match aggregat_sperrgrund(sensoren) {' '        let aggregat = match None::<String> { // ROTBEWEIS M-44: Sperrgrund nicht gefragt'))))

$mutationen.Add((Mutation 'M-45' '(1)' 'Weckruf im Stoppweg entfernt' 'lib:stopp_weckt_ohne_wartedurchlauf' 'M-45 \(1\) stopp_weckt_ohne_wartedurchlauf: Wartedurchlaeufe nach dem Stoppsignal 1 statt 0' 'NAK-286-rot-M-45.txt' @(
    (Ersetzung $BK '            kern.weckruf.notify_all(); // der Stoppweg weckt (M-45 (1))' '            // ROTBEWEIS M-45 (1): Weckruf im Stoppweg entfernt'))))

$mutationen.Add((Mutation 'M-45' '(2)' 'Stoppflag im Takt nicht geprueft' 'lib:nach_stopp_keine_lieferung' 'M-45 \(2\) nach_stopp_keine_lieferung: Existenzpruefungen nach dem Stopp' 'NAK-286-rot-M-45.txt' @(
    (Ersetzung $BK @'
            if weck.stopp {
                break;
            }
'@ @'
            // ROTBEWEIS M-45 (2): Stoppflag im Takt nicht geprueft
'@))))

$mutationen.Add((Mutation 'M-45' '(3)' 'Briefkasten nach dem Store gestoppt (lebenslauf.rs voruebergehend mutiert)' 'lib:stoppreihenfolge_briefkasten_zuerst' 'M-45 \(3\) stoppreihenfolge_briefkasten_zuerst: Protokoll der gemeinsamen Stoppfunktion' 'NAK-286-rot-M-45.txt' @(
    (Ersetzung $LL @'
    drop(entnehmen(briefkasten));
    drop(entnehmen(supervisor));
'@ @'
    drop(entnehmen(supervisor));
'@),
    (Ersetzung $LL '    drop(entnehmen(store));' @'
    drop(entnehmen(store));
    drop(entnehmen(briefkasten)); // ROTBEWEIS M-45 (3): Briefkasten nach dem Store gestoppt
'@))))

$mutationen.Add((Mutation 'M-46' '' 'Briefkastenstart in probe_lauf (probe.rs voruebergehend mutiert)' 'int:probe_lauf_startet_keinen_briefkasten' 'M-46 probe_lauf_startet_keinen_briefkasten: Startzaehler 1 statt 0' 'NAK-286-rot-M-46.txt' @(
    (Ersetzung $PR '    std::thread::sleep(std::time::Duration::from_secs(sekunden));' @'
    let _briefkasten = crate::briefkasten::briefkasten_starten(register.clone(), &session_token); // ROTBEWEIS M-46: Briefkastenstart in probe_lauf
    std::thread::sleep(std::time::Duration::from_secs(sekunden));
'@))))

$mutationen.Add((Mutation 'M-47' '' 'Schreiben innerhalb des Sperrbereichs des Register-Mutex' 'lib:register_sperre_nie_ueber_dateischreiben' 'M-47 register_sperre_nie_ueber_dateischreiben: Register-Mutex im Schreibschritt nicht frei' 'NAK-286-rot-M-47.txt' @(
    (Ersetzung $BK @'
        let (sensoren, token_kurz) = {
            let register = sperre(&self.register);
'@ @'
        let register = sperre(&self.register); // ROTBEWEIS M-47: Schreiben innerhalb des Sperrbereichs
        let (sensoren, token_kurz) = {
'@))))

$mutationen.Add((Mutation 'M-48' '' 'Kennungsmuster auf 32 beliebige Zeichen gelockert' 'lib:pfadtraversal_unmoeglich' 'M-48 pfadtraversal_unmoeglich \.\.\\waechter\\ \(32 Zeichen\)' 'NAK-286-rot-M-48-rust.txt' @(
    (Ersetzung $BK "    zeichen.len() == 32 && zeichen.iter().all(|z| matches!(z, b'0'..=b'9' | b'a'..=b'f'))" '    zeichen.len() == 32 // ROTBEWEIS M-48: Kennungsmuster auf 32 beliebige Zeichen gelockert'))))

$mutationen.Add((Mutation 'M-49' '' 'Attributpruefung des Reparse-Punkts entfernt' 'lib:reparse_punkte_werden_ignoriert' 'M-49 reparse_punkte_werden_ignoriert \(a\) antwort ist eine junction' 'NAK-286-rot-M-49-rust.txt' @(
    (Ersetzung $BK '    attribute.reparse' '    false && attribute.reparse // ROTBEWEIS M-49: Attributpruefung entfernt'))))

$mutationen.Add((Mutation 'M-51' '(1)' 'Ringgroesse: Ring mit einem Eintrag' 'lib:kennungsring_fifo_256' 'M-51 kennungsring_fifo_256 zweitjuengste erneut 0 anlegeversuche' 'NAK-286-rot-M-51-rust.txt' @(
    (Ersetzung $BK 'const KENNUNGSRING: usize = 256;' 'const KENNUNGSRING: usize = 1; // ROTBEWEIS M-51 (1): Ring mit einem Eintrag'))))

$mutationen.Add((Mutation 'M-51' '(2)' 'Einmaligkeit: Ring vor dem Schreiben nicht gefragt' 'lib:kennungsring_fifo_256' 'M-51 kennungsring_fifo_256 zweitjuengste erneut 0 anlegeversuche' 'NAK-286-rot-M-51-rust.txt' @(
    (Ersetzung $BK '        if zustand.ring_enthaelt(&kennung) {' '        if false && zustand.ring_enthaelt(&kennung) { // ROTBEWEIS M-51 (2): Ring vor dem Schreiben nicht gefragt'))))

$mutationen.Add((Mutation 'M-52' '' 'Zuordnung ueber "neuer als die letzte Antwort" nach der Uhr' 'lib:uhrsprung_aendert_zuordnung_nicht' 'M-52 uhrsprung_aendert_zuordnung_nicht' 'NAK-286-rot-M-52-rust.txt' @(
    (Ersetzung $BK '        zustand.offen.versuche += 1;' @'
        static LETZTE_ANTWORT_MS: std::sync::atomic::AtomicI64 = std::sync::atomic::AtomicI64::new(i64::MIN); // ROTBEWEIS M-52
        let jetzt_uhr = self.fassaden.uhr.jetzt_utc_ms();
        if jetzt_uhr <= LETZTE_ANTWORT_MS.load(Ordering::SeqCst) {
            return; // ROTBEWEIS M-52: nur neuer als die letzte Antwort nach der Uhr
        }
        LETZTE_ANTWORT_MS.store(jetzt_uhr, Ordering::SeqCst);
        zustand.offen.versuche += 1;
'@))))

$mutationen.Add((Mutation 'M-53' '(a)' 'direkt in den Antwortnamen geschrieben' 'lib:keine_leere_antwort' 'M-53 keine_leere_antwort \(a\) fehler vor dem ersten byte' 'NAK-286-rot-M-53-rust.txt' @(
    (Ersetzung $BK '        let temp = Pfad::neu(self.ebenen[3].pfad.join(format!("{name}.tmp-{}", self.pid)));' '        let temp = Pfad::neu(self.ebenen[3].pfad.join(&name)); // ROTBEWEIS M-53 (a): direkt in den Antwortnamen'))))

$mutationen.Add((Mutation 'M-53' '(b)' 'Aufraeumer loescht die Temp-Datei, wenn das Umbenennen scheitert' 'lib:keine_leere_antwort' 'M-53 keine_leere_antwort \(b\) fehler beim umbenennen' 'NAK-286-rot-M-53-rust.txt' @(
    (Ersetzung $BK @'
        dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel)
    }
'@ @'
        if dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel) {
            return true;
        }
        dateisystem.loesche(&temp); // ROTBEWEIS M-53 (b): Aufraeumer loescht die Temp-Datei
        false
    }
'@))))

function Fahre ([string] $test, [string] $ausgabe)
{
    $art, $name = $test -split ':', 2
    $argumente = if ($art -eq 'int') { @('test', '--manifest-path', 'broker/Cargo.toml', '--test', 'briefkasten', '--color', 'never', '--', $name, '--exact', '--nocapture') }
                 else { @('test', '--manifest-path', 'broker/Cargo.toml', '--lib', '--color', 'never', '--', "briefkasten::tests::$name", '--exact', '--nocapture') }
    & cargo @argumente *> $ausgabe
    return [pscustomobject]@{ Exit = $LASTEXITCODE; Befehl = 'cargo ' + ($argumente -join ' ') }
}

# Mit `pwsh -File` kommt `-Nur M-41,M-45` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($mutationen | Where-Object { $Nur.Count -eq 0 -or $Nur -contains $_.Zeile })
$zusammenfassung = New-Object System.Collections.Generic.List[string]

foreach ($m in $auswahl)
{
    $name = ($m.Zeile + ' ' + $m.Teil).Trim()
    $dateien = @($m.Ersetzungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $orig = @{}; $shaVor = @{}; $texte = @{}
    foreach ($d in $dateien)
    {
        $orig[$d] = [IO.File]::ReadAllBytes((Join-Path $wurzel $d))
        $shaVor[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
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
    if ($m.Zeile -eq 'M-46')
    {
        $echt = Join-Path $env:LOCALAPPDATA 'evenacadia\nakama\diagnose\anfrage.json'
        if (Test-Path $echt) { throw "M-46 verweigert: $echt liegt; der mutierte probe_lauf wuerde sie beantworten" }
    }

    $shaMut = @{}
    $rot = $null; $rotZeilen = @(); $traeger = @()
    $ausRot = Join-Path $tmp 'a4-rot.txt'
    try
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $utf8.GetBytes($texte[$d]))
            $shaMut[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        }
        $rot = Fahre $m.Test $ausRot
        if (Test-Path $ausRot)
        {
            $rotZeilen = @(Get-Content $ausRot)
            $traeger = @($rotZeilen | Where-Object { $_ -match $m.Traeger })
        }
    }
    finally
    {
        foreach ($d in $dateien) { [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $orig[$d]) }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $ausGruen = Join-Path $tmp 'a4-gruen.txt'
    $gruen = Fahre $m.Test $ausGruen
    $summeGruen = if (Test-Path $ausGruen) { (Get-Content $ausGruen | Where-Object { $_ -like 'test result:*' } | Select-Object -Last 1) } else { '' }

    $gruende = @()
    if ($null -eq $rot -or $rot.Exit -eq 0) { $gruende += "Lauf rot Exit $(if ($rot) { $rot.Exit } else { 'fehlt' })" }
    if ($traeger.Count -lt 1) { $gruende += 'keine Traegerzeile' }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if ($gruen.Exit -ne 0) { $gruende += "Lauf gruen Exit $($gruen.Exit)" }
    $ergebnis = if ($gruende.Count -eq 0) { 'ROTBEWEIS GUELTIG' } else { 'ROTBEWEIS UNGUELTIG (' + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/' + $m.Roh)
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 3, Rotbeweis $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Mutation: $($m.Beschreibung)")
    $zeilen.Add("Bein A4 als Einzeltest: $($rot.Befehl)")
    $zeilen.Add("Traeger (Muster der Meldung): $($m.Traeger)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })")
    }
    $zeilen.Add("A4 rot: Exit $($rot.Exit), Traegerzeilen $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add('--- Rohausgabe A4, mutierter Stand ---')
    foreach ($z in $rotZeilen) { $zeilen.Add($z) }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    $zeilen.Add("A4 gruen: Exit $($gruen.Exit), $summeGruen")
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
