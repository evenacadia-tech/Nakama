# NAK-283 Etappe 3 - Rotbeweise der Matrixzeilen M-13 bis M-26 und M-75
# (Bauer, 12.09.2026)
#
# Ablauf je Mutation nach docs/beweise/NAK-283.md §6.1:
#   1. SHA-256 der unveraenderten Quelldatei notieren, Originalbytes sichern
#   2. Mutation GENAU an der Zeile, die die Zusage traegt (jedes Muster muss
#      genau einmal vorkommen, sonst Abbruch vor der Messung)
#   3. LastWriteTime setzen und die betroffenen Testziele bauen (`--no-run`);
#      die Bauausgabe muss `Compiling eqcop-broker` tragen - sonst hat cargo
#      nicht neu uebersetzt, und der Lauf maesse die alte Fassung
#   4. je Matrixzeile den Fall fahren -> erwartet ROT, und zwar an der
#      Zusagezeile: die tragende Meldung muss in JEDEM Rotlauf fallen
#   5. Originalbytes zurueckspielen (nie eine zweite Textersetzung), SHA-256
#      gegen Schritt 1, LastWriteTime setzen, neu bauen, Fall fahren -> GRUEN
#
# Nebenlaeufigkeitszeilen (M-13a, M-13b, M-75, M-16a, M-16b, M-17, M-20b, M-21,
# M-23, M-24, M-25a, M-25b, M-26 - der Auftrag und §6.1 zusammen) fahren DREI
# Rotlaeufe; das Interleaving erzwingt ein Haken, nie Schlaf oder Zeit.
# MUT-16c bricht die Obergrenze von „genau eine Neurechnung" (ein Rotlauf,
# deterministisch).
#
# M-75 sichert zusaetzlich seinen FREIGABEGRUND zu: jeder Rotlauf muss
# `Freigabegrund = Signal` melden (B kam durch die Luecke), der Gruenlauf
# `Freigabegrund = Frist`. Meldet ein Rotlauf `Frist`, ist er NICHT GEMESSEN und
# wird mit laengerer Frist (NAK283_M75_FRIST_MS = 4000, dann 15000) wiederholt;
# er zaehlt nie als rot.
#
# Regressionswachen (M-14, M-15, M-18, M-19, M-20a, M-20b, M-22) werden hier
# absichtlich gebrochen und bleiben als Wachen benannt - sie sind kein Beleg
# fuer ihren Befund.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -NoProfile -File docs/beweise/roh/NAK-283-etappe-3-rotskript.ps1
#   pwsh -NoProfile -File docs/beweise/roh/NAK-283-etappe-3-rotskript.ps1 -Nur MUT-75
#
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-3.txt

[CmdletBinding()]
param(
    [string] $Nur = '',
    # Nur die Mutationsmuster pruefen (genau ein Treffer je Paar, in Reihenfolge
    # angewandt) - liest die Quellen, schreibt nichts, baut nichts.
    [switch] $Trocken,
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
)

$ErrorActionPreference = 'Stop'
Set-Location $Wurzel
$rohOrdner = Join-Path $Wurzel 'docs\beweise\roh'

function Zeilenende([string] $text) {
    if ($text -match "`r`n") { return "`r`n" } else { return "`n" }
}

# Die Muster werden zeilenweise gesucht und mit dem Zeilenende der DATEI
# verbunden - der Baum ist gemischt (LF und CRLF), CLAUDE.md gitattributes.
function Mutiere([string] $datei, [object[]] $paare) {
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    foreach ($p in $paare) {
        $suche = ($p.Alt -join $le)
        $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
        if ($treffer -ne 1) {
            throw "Mutationsmuster $treffer-mal gefunden in $datei (erwartet: 1): $($p.Alt[0])"
        }
        $t = $t.Replace($suche, ($p.Neu -join $le))
    }
    [System.IO.File]::WriteAllText($datei, $t, (New-Object System.Text.UTF8Encoding $false))
}

function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }

# ACHTUNG: PowerShell loest Befehlsnamen ohne Ruecksicht auf Gross- und
# Kleinschreibung auf, und eine FUNKTION geht einer Anwendung vor. Eine
# Funktion namens `Cargo` rief in `& cargo` sich selbst auf - der erste Lauf
# endete mit einem Ueberlauf der Aufruftiefe, bevor irgendetwas mutiert war.
function CargoLauf([string[]] $argumente) {
    $aus = @(& cargo.exe @argumente 2>&1 | ForEach-Object { "$_" })
    return [pscustomobject]@{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Baue([string[]] $ziele) {
    $texte = @()
    foreach ($z in $ziele) {
        $argumente = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never', '--no-run')
        if ($z -eq 'lib') { $argumente += '--lib' } else { $argumente += @('--test', $z) }
        $r = CargoLauf $argumente
        if ($r.ExitCode -ne 0) { throw "Bau von $z fehlgeschlagen:`n$($r.Text)" }
        $texte += ("[{0}] {1}" -f $z, (($r.Text -split "`n" | Where-Object { $_ -match 'Compiling|Finished|warning: unused|never constructed|never used' }) -join ' | '))
        if ($r.Text -notmatch 'Compiling eqcop-broker') { throw "Kein Neubau von $z - cargo hat die geaenderte Quelle nicht uebersetzt" }
    }
    return ($texte -join "`n")
}

function Fahre([string] $ziel, [string] $test, [string] $frist) {
    $argumente = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never')
    if ($ziel -eq 'lib') { $argumente += '--lib' } else { $argumente += @('--test', $ziel) }
    $argumente += @($test, '--', '--exact', '--nocapture')
    # Eine auf $null gesetzte Umgebungsvariable ist in PowerShell 7 entfernt.
    if ($frist) { $env:NAK283_M75_FRIST_MS = $frist } else { $env:NAK283_M75_FRIST_MS = $null }
    try { return CargoLauf $argumente }
    finally { $env:NAK283_M75_FRIST_MS = $null }
}

function Freigabegrund([string] $text) {
    $m = [regex]::Match($text, 'Freigabegrund = (Signal|Frist)')
    if ($m.Success) { return $m.Groups[1].Value }
    return ''
}

function Faellt([string] $text, [string[]] $erwartet) {
    foreach ($e in $erwartet) { if ($text.Contains($e)) { return $e } }
    return ''
}

$hyp = Join-Path $Wurzel 'broker\src\coordinator\hypothese_verdrahtung.rs'
$inv = Join-Path $Wurzel 'broker\src\coordinator\invalidierung_verdrahtung.rs'
$que = Join-Path $Wurzel 'broker\src\transport\server_v3\queues.rs'
$s14 = 'sonde014_verdrahtung'
$s13 = 'sonde013_verdrahtung'
$qt  = 'transport::server_v3::queues::tests::'

# Die Erwartungshashes: der Stand VOR dem Skript. Am Ende muss jede Datei
# bytegleich dazu sein.
$ausgang = @{}
foreach ($d in @($hyp, $inv, $que)) { $ausgang[$d] = Hash-Von $d }

$ersetzungAlt = @(
    '                    ersetzt = g.0.remove(position);',
    '                    g.0.push_back(eintrag);')

$mutationen = @(
    [pscustomobject]@{
        Kennung = 'MUT-13a'; Datei = $hyp
        Was = 'ergebnis_ist_noch_gueltig: der Vergleich der Eingangsmenge entfaellt (Stand am Basis-SHA: nur Generation und verwendete IDs)'
        Paare = @(@{
            Alt = @('        if gueltig != *eingangsmenge {', '            return Rueckschreibung::Ueberholt;', '        }')
            Neu = @('        // MUTATION NAK-283 M-13a: Vergleich der Eingangsmenge entfernt') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-13a'; Ziel = $s14; Test = 'neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis'; Rot = 3; Datei = 'NAK-283-rot-M-13a-etappe-3.txt'
               Erwartet = @('M-13 Fenster 1: neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis') },
            @{ Zeile = 'M-26 (Haelfte 2)'; Ziel = $s14; Test = 'neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis'; Rot = 3; Datei = 'NAK-283-rot-M-26-etappe-3.txt'
               Erwartet = @('M-13 Fenster 1: neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis') },
            @{ Zeile = 'M-25a'; Ziel = $s14; Test = 'kein_rueckschreibeweg_des_coordinators_benennt_sein_ziel_ueber_eine_position'; Rot = 3; Datei = 'NAK-283-rot-M-25a-etappe-3.txt'
               Erwartet = @('M-25 Weg 1: kein_rueckschreibeweg_des_coordinators_benennt_sein_ziel_ueber_eine_position') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-13b'; Datei = $hyp
        Was = 'befund_persistieren: der Wiedervergleich entfaellt, die Sperre bleibt unberuehrt'
        Paare = @(@{
            Alt = @('            if !Self::standeintrag_ist_noch(&stand, session, eingetragen) {')
            Neu = @('            if false { // MUTATION NAK-283 M-13b: Wiedervergleich entfernt, die Sperre bleibt') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-13b'; Ziel = $s14; Test = 'aelterer_payload_wird_nach_dem_cacheeintrag_nicht_persistiert'; Rot = 3; Datei = 'NAK-283-rot-M-13b-etappe-3.txt'
               Erwartet = @('M-13 Fenster 2: aelterer_payload_wird_nach_dem_cacheeintrag_nicht_persistiert') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-75'; Datei = $hyp
        Was = 'befund_persistieren: der Guard faellt zwischen bestandenem Vergleich und append_einreihen (die Fassung der Nacharbeit 1); der Haken feuert ohne Guard, die Annahme nimmt den Standlock neu'
        Paare = @(@{
            Alt = @(
                '            if let Some(haken) = annahme_haken.as_ref() {',
                '                haken.erreichen();',
                '            }',
                '            match store.append_einreihen(vec![event]) {')
            Neu = @(
                '            drop(stand); // MUTATION NAK-283 M-75: Guard zwischen bestandenem Vergleich und Annahme frei',
                '            if let Some(haken) = annahme_haken.as_ref() {',
                '                haken.erreichen();',
                '            }',
                '            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner()); // MUTATION NAK-283 M-75',
                '            match store.append_einreihen(vec![event]) {') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-75'; Ziel = $s14; Test = 'annahmeordnung_folgt_der_vergleichsordnung'; Rot = 3; Datei = 'NAK-283-rot-M-75-etappe-3.txt'; Freigabegrund = $true
               Erwartet = @('M-75: annahmeordnung_folgt_der_vergleichsordnung') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-16a'; Datei = $hyp
        Was = 'befunde_eintragen, Verwurfzweig Ueberholt (Fenster 1): befunde_neu_bilden wird nicht gesetzt'
        Paare = @(@{
            Alt = @('                    // ihn im Heilungstakt sofort ein (M-16).', '                    stand.befunde_neu_bilden = true;')
            Neu = @('                    // ihn im Heilungstakt sofort ein (M-16).', '                    // MUTATION NAK-283 M-16a: Merker im Verwurfzweig vor dem Cache-Eintrag nicht gesetzt') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-16a'; Ziel = $s14; Test = 'verworfene_rechnung_zieht_genau_eine_neurechnung_nach'; Rot = 3; Datei = 'NAK-283-rot-M-16a-etappe-3.txt'
               Erwartet = @('M-16 Fenster 1: verworfene_rechnung_zieht_genau_eine_neurechnung_nach') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-16b'; Datei = $hyp
        Was = 'befund_persistieren, unterlassener Schreibversuch (Fenster 2): befunde_neu_bilden wird nicht gesetzt'
        Paare = @(@{
            Alt = @('                    stand.befund_schreibversuche_unterlassen.saturating_add(1);', '                stand.befunde_neu_bilden = true;', '                return false;')
            Neu = @('                    stand.befund_schreibversuche_unterlassen.saturating_add(1);', '                // MUTATION NAK-283 M-16b: Merker im Verwurfzweig vor der Persistenz nicht gesetzt', '                return false;') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-16b'; Ziel = $s14; Test = 'verworfene_rechnung_zieht_genau_eine_neurechnung_nach'; Rot = 3; Datei = 'NAK-283-rot-M-16b-etappe-3.txt'
               Erwartet = @('M-16 Fenster 2: verworfene_rechnung_zieht_genau_eine_neurechnung_nach') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-16c'; Datei = $hyp
        Was = 'hypothesen_bilden: nach dem Heilungstakt laeuft eine zweite Rechnung - bricht die Obergrenze von genau einer Neurechnung (Zaehlhaken stellt sich neu scharf)'
        Paare = @(@{
            Alt = @('        if self.hypothesen_rechnen() {', '            self.hypothesen_bei_bedarf_bilden();', '        }')
            Neu = @('        if self.hypothesen_rechnen() {', '            self.hypothesen_bei_bedarf_bilden();', '            self.hypothesen_rechnen(); // MUTATION NAK-283 M-16c: zweite Rechnung nach dem Heilungstakt', '        }') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-16c'; Ziel = $s14; Test = 'verworfene_rechnung_zieht_genau_eine_neurechnung_nach'; Rot = 1; Datei = 'NAK-283-rot-M-16c-etappe-3.txt'
               Erwartet = @('M-16 Fenster 1: der Verwurf vor dem Cache-Eintrag zieht GENAU EINE Neurechnung nach sich') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-14'; Datei = $hyp
        Was = 'gueltige_evidenz_ids_locked: der Riegel verliert den ID-Gueltigkeitsteil (ausgeschlossene Belege gelten als gueltig) - Regressionswache'
        Paare = @(@{
            Alt = @('            for eintrag in historie.iter() {', '                if eintrag.ausschlussgrund.is_none() {', '                    gueltig.insert(eintrag.evidence_id.clone());')
            Neu = @('            for eintrag in historie.iter() {', '                { // MUTATION NAK-283 M-14: Ausschluss wird nicht mehr geprueft', '                    gueltig.insert(eintrag.evidence_id.clone());') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-14'; Ziel = $s14; Test = 'veraltetes_rechenergebnis_wird_nicht_veroeffentlicht'; Rot = 1; Datei = 'NAK-283-rot-M-14-etappe-3.txt'
               Erwartet = @('eine waehrend der Rechnung zurueckgenommene ID darf nie wieder sichtbar werden') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-15'; Datei = $hyp
        Was = 'ergebnis_ist_noch_gueltig: die Generationspruefung entfaellt - Regressionswache'
        Paare = @(@{
            Alt = @(
                '            if befunde',
                '                .iter()',
                '                .any(|b| b.intent_generation != aktuelle_generation)',
                '            {',
                '                return Rueckschreibung::Veraltet;',
                '            }')
            Neu = @('            // MUTATION NAK-283 M-15: Generationspruefung entfernt') })
        Ziele = @($s14)
        Laeufe = @(
            @{ Zeile = 'M-15'; Ziel = $s14; Test = 'veraltetes_rechenergebnis_wird_nicht_veroeffentlicht'; Rot = 1; Datei = 'NAK-283-rot-M-15-etappe-3.txt'
               Erwartet = @('kein READY-Befund der ALTEN Revision im Snapshot', 'der bereits veraltete Bestand steht weiter - er wurde nicht ueberschrieben') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-17'; Datei = $que
        Was = 'einreihen_eintrag: die Ersetzung wieder an der alten Position (std::mem::replace, Stand am Basis-SHA)'
        Paare = @(@{ Alt = $ersetzungAlt
            Neu = @('                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag)); // MUTATION NAK-283 M-17: Ersetzung an der alten Position') })
        Ziele = @('lib')
        Laeufe = @(
            @{ Zeile = 'M-17'; Ziel = 'lib'; Test = ($qt + 'koaleszierung_erhaelt_die_ordnung_ueber_schluessel'); Rot = 3; Datei = 'NAK-283-rot-M-17-etappe-3.txt'
               Erwartet = @('M-17: koaleszierung_erhaelt_die_ordnung_ueber_schluessel') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-18'; Datei = $que
        Was = 'einreihen_eintrag: statt zu ersetzen wird angehaengt (zwei Eintraege desselben Schluessels) - Regressionswache fuer M-18 und M-20 Haelfte 1'
        Paare = @(@{ Alt = $ersetzungAlt
            Neu = @('                    g.0.push_back(eintrag); // MUTATION NAK-283 M-18/M-20a: anhaengen statt ersetzen') })
        Ziele = @('lib')
        Laeufe = @(
            @{ Zeile = 'M-18'; Ziel = 'lib'; Test = ($qt + 'writerqueue_snapshot_koalesziert_nach_objektschluessel'); Rot = 1; Datei = 'NAK-283-rot-M-18-etappe-3.txt'
               Erwartet = @('called `Result::unwrap()` on an `Err` value: Timeout') },
            @{ Zeile = 'M-20a'; Ziel = 'lib'; Test = ($qt + 'ersetzung_ans_ende_laesst_die_queue_nicht_wachsen'); Rot = 1; Datei = 'NAK-283-rot-M-20a-etappe-3.txt'
               Erwartet = @('M-20 Haelfte 1: ersetzung_ans_ende_laesst_die_queue_nicht_wachsen') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-19'; Datei = $que
        Was = 'einreihen_eintrag: die Hochwasserpruefung mit marke <= hochwasser (Gleichheit wird Nachzuegler) - Regressionswache'
        Paare = @(@{
            Alt = @('                if hochwasser.is_some_and(|hoch| eintrag.marke < hoch) {')
            Neu = @('                if hochwasser.is_some_and(|hoch| eintrag.marke <= hoch) { // MUTATION NAK-283 M-19') })
        Ziele = @('lib')
        Laeufe = @(
            @{ Zeile = 'M-19'; Ziel = 'lib'; Test = ($qt + 'aelterer_nachzuegler_ersetzt_keinen_neueren_snapshot'); Rot = 1; Datei = 'NAK-283-rot-M-19-etappe-3.txt'
               Erwartet = @('M-21: gleiche Marke 12 wird angenommen') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-20b'; Datei = $que
        Was = 'einreihen_eintrag: Entfernen und Anhaengen in ZWEI Sperrabschnitten, der Ersetzungshaken feuert dazwischen - Regressionswache'
        Paare = @(@{ Alt = $ersetzungAlt
            Neu = @(
                '                    ersetzt = g.0.remove(position);',
                '                    drop(g); // MUTATION NAK-283 M-20b: Entfernen und Anhaengen in zwei Sperrabschnitten',
                '                    #[cfg(test)]',
                '                    self.ersetzungshaken_ausloesen();',
                '                    g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());',
                '                    g.0.push_back(eintrag);') })
        Ziele = @('lib')
        Laeufe = @(
            @{ Zeile = 'M-20b'; Ziel = 'lib'; Test = ($qt + 'konkurrierendes_einreihen_findet_kein_fenster_ohne_schluessel'); Rot = 3; Datei = 'NAK-283-rot-M-20b-etappe-3.txt'
               Erwartet = @('M-20 Haelfte 2: konkurrierendes_einreihen_findet_kein_fenster_ohne_schluessel') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-21'; Datei = $inv
        Was = 'Invalidierungswirkung.zurueck wieder ueber den Deque-Index: Vorbereitung merkt die Position, Ruecknahme greift ueber get_mut(index) (Stand am Basis-SHA)'
        Paare = @(
            @{ Alt = @('    zurueck: Vec<(ClientKey, String)>,')
               Neu = @('    zurueck: Vec<(ClientKey, usize)>, // MUTATION NAK-283 M-21: Deque-Index statt stabiler ID') },
            @{ Alt = @('        let mut zurueck: Vec<(ClientKey, String)> = Vec::new();')
               Neu = @('        let mut zurueck: Vec<(ClientKey, usize)> = Vec::new();') },
            @{ Alt = @('            for eintrag in historie.iter_mut() {')
               Neu = @('            for (index, eintrag) in historie.iter_mut().enumerate() {') },
            @{ Alt = @('                    zurueck.push((key.clone(), eintrag.evidence_id.clone()));')
               Neu = @('                    zurueck.push((key.clone(), index));') },
            @{ Alt = @(
                   '        for (key, evidence_id) in &wirkung.zurueck {',
                   '            let Some(historie) = stand.evidenz.get_mut(key) else {',
                   '                continue;',
                   '            };',
                   '            let Some(eintrag) = historie',
                   '                .iter_mut()',
                   '                .find(|eintrag| eintrag.evidence_id == *evidence_id)',
                   '            else {',
                   '                // Von der Retention entfernt: uebersprungen.',
                   '                continue;',
                   '            };')
               Neu = @(
                   '        for (key, index) in &wirkung.zurueck {',
                   '            let Some(historie) = stand.evidenz.get_mut(key) else {',
                   '                continue;',
                   '            };',
                   '            // MUTATION NAK-283 M-21: Ruecknahme ueber den Deque-Index',
                   '            let Some(eintrag) = historie.get_mut(*index) else {',
                   '                continue;',
                   '            };') })
        Ziele = @($s13, $s14)
        Laeufe = @(
            @{ Zeile = 'M-21'; Ziel = $s13; Test = 'rollback_ueber_stabile_evidence_ids_laesst_fremden_ausschluss_stehen'; Rot = 3; Datei = 'NAK-283-rot-M-21-etappe-3.txt'
               Erwartet = @('M-21: rollback_ueber_stabile_evidence_ids_laesst_fremden_ausschluss_stehen') },
            @{ Zeile = 'M-24'; Ziel = $s13; Test = 'cache_und_projektion_sind_nach_dem_storefehler_gleich'; Rot = 3; Datei = 'NAK-283-rot-M-24-etappe-3.txt'
               Erwartet = @('M-24: cache_und_projektion_sind_nach_dem_storefehler_gleich') },
            @{ Zeile = 'M-25b'; Ziel = $s14; Test = 'kein_rueckschreibeweg_des_coordinators_benennt_sein_ziel_ueber_eine_position'; Rot = 3; Datei = 'NAK-283-rot-M-25b-etappe-3.txt'
               Erwartet = @('M-25 Weg 2: kein_rueckschreibeweg_des_coordinators_benennt_sein_ziel_ueber_eine_position') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-22'; Datei = $inv
        Was = 'invalidierung_ruecknehmen: der Rollback bricht nach dem ersten Treffer ab - Regressionswache'
        Paare = @(@{
            Alt = @('            eintrag.ausschlussgrund = None;')
            Neu = @('            eintrag.ausschlussgrund = None;', '            break; // MUTATION NAK-283 M-22: Rollback bricht nach dem ersten Treffer ab') })
        Ziele = @($s13)
        Laeufe = @(
            @{ Zeile = 'M-22'; Ziel = $s13; Test = 'rollback_nimmt_den_eigenen_ausschluss_vollstaendig_zurueck'; Rot = 1; Datei = 'NAK-283-rot-M-22-etappe-3.txt'
               Erwartet = @('M-22: rollback_nimmt_den_eigenen_ausschluss_vollstaendig_zurueck') })
    }
    [pscustomobject]@{
        Kennung = 'MUT-23'; Datei = $inv
        Was = 'invalidierung_ruecknehmen: der Uebersprung-Zweig entfaellt - eine unbekannte ID bricht den Rollback ab'
        Paare = @(@{
            Alt = @('                // Von der Retention entfernt: uebersprungen.', '                continue;')
            Neu = @('                // Von der Retention entfernt: uebersprungen.', '                break; // MUTATION NAK-283 M-23: unbekannte ID bricht den Rollback ab') })
        Ziele = @($s13)
        Laeufe = @(
            @{ Zeile = 'M-23'; Ziel = $s13; Test = 'retention_am_deckel_verschiebt_keine_rollbackzuordnung'; Rot = 3; Datei = 'NAK-283-rot-M-23-etappe-3.txt'
               Erwartet = @('M-23 Stufe 32: retention_am_deckel_verschiebt_keine_rollbackzuordnung') })
    }
)

if ($Trocken) {
    $fehler = 0
    foreach ($m in $mutationen) {
        if ($Nur -and $m.Kennung -ne $Nur) { continue }
        $t = [System.IO.File]::ReadAllText($m.Datei)
        $le = Zeilenende $t
        $i = 0
        foreach ($p in $m.Paare) {
            $i++
            $suche = ($p.Alt -join $le)
            $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
            $ok = ($treffer -eq 1)
            if (-not $ok) { $fehler++ }
            Write-Host ("{0} Paar {1}: {2} Treffer - {3}" -f $m.Kennung, $i, $treffer, $(if ($ok) { 'ok' } else { 'FEHLER' }))
            if ($ok) { $t = $t.Replace($suche, ($p.Neu -join $le)) }
        }
    }
    if ($fehler -gt 0) {
        Write-Host "TROCKENLAUF: $fehler Muster ohne genau einen Treffer" -ForegroundColor Red
        exit 1
    }
    Write-Host 'TROCKENLAUF: alle Muster genau einmal gefunden' -ForegroundColor Green
    exit 0
}

$bilanz = New-Object System.Collections.Generic.List[object]
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ===" -ForegroundColor Cyan

    $sicherung = [System.IO.File]::ReadAllBytes($m.Datei)
    $hashVorher = Hash-Von $m.Datei
    if ($hashVorher -ne $ausgang[$m.Datei]) { throw "Quell-Hash von $($m.Datei) weicht vom Ausgangsstand ab - vorige Ruecknahme nicht bytegleich?" }

    $ergebnisse = @{}
    $bauRot = ''
    try {
        Mutiere $m.Datei $m.Paare
        (Get-Item $m.Datei).LastWriteTime = Get-Date
        $bauRot = Baue $m.Ziele
        foreach ($l in $m.Laeufe) {
            $laeufe = @()
            $gemessen = 0
            $versuche = 0
            while ($gemessen -lt $l.Rot) {
                $versuche++
                if ($versuche -gt ($l.Rot + 6)) { break }
                $frist = ''
                $r = Fahre $l.Ziel $l.Test $frist
                $grund = Freigabegrund $r.Text
                if ($l.Freigabegrund -and $grund -eq 'Frist') {
                    foreach ($laenger in @('4000', '15000')) {
                        $laeufe += [pscustomobject]@{ Nr = $versuche; Frist = $(if ($frist) { $frist } else { 'Standard 1000' }); Exit = $r.ExitCode; Grund = $grund; Faellt = (Faellt $r.Text $l.Erwartet); Gemessen = $false; Text = $r.Text }
                        $frist = $laenger
                        $r = Fahre $l.Ziel $l.Test $frist
                        $grund = Freigabegrund $r.Text
                        if ($grund -ne 'Frist') { break }
                    }
                }
                $faellt = Faellt $r.Text $l.Erwartet
                $istGemessen = ($r.ExitCode -ne 0) -and ($faellt -ne '') -and ((-not $l.Freigabegrund) -or $grund -eq 'Signal')
                if ($istGemessen) { $gemessen++ }
                $laeufe += [pscustomobject]@{ Nr = $versuche; Frist = $(if ($frist) { $frist } else { 'Standard' }); Exit = $r.ExitCode; Grund = $grund; Faellt = $faellt; Gemessen = $istGemessen; Text = $r.Text }
                Write-Host ("  ROT   {0} Versuch {1}: Exit {2}, Zusagezeile {3}{4}" -f $l.Zeile, $versuche, $r.ExitCode, $(if ($faellt) { 'faellt' } else { 'FAELLT NICHT' }), $(if ($l.Freigabegrund) { ", Freigabegrund $grund" } else { '' }))
                if (-not $istGemessen -and -not ($l.Freigabegrund -and $grund -eq 'Frist')) { break }
            }
            $ergebnisse[$l.Datei] = [pscustomobject]@{ Lauf = $l; Laeufe = $laeufe; Gemessen = $gemessen }
        }
    }
    finally {
        [System.IO.File]::WriteAllBytes($m.Datei, $sicherung)
    }
    $hashNachher = Hash-Von $m.Datei
    if ($hashNachher -ne $hashVorher) { throw "Ruecknahme NICHT bytegleich in $($m.Datei)" }
    (Get-Item $m.Datei).LastWriteTime = Get-Date
    $bauGruen = Baue $m.Ziele

    foreach ($l in $m.Laeufe) {
        $e = $ergebnisse[$l.Datei]
        $gruen = Fahre $l.Ziel $l.Test ''
        $gruenGrund = Freigabegrund $gruen.Text
        $gruenOk = ($gruen.ExitCode -eq 0) -and ((-not $l.Freigabegrund) -or $gruenGrund -eq 'Frist')
        Write-Host ("  GRUEN {0}: Exit {1}{2}" -f $l.Zeile, $gruen.ExitCode, $(if ($l.Freigabegrund) { ", Freigabegrund $gruenGrund" } else { '' }))

        $inhalt = New-Object System.Collections.Generic.List[string]
        foreach ($z in @(
            "NAK-283 Etappe 3 - Rotbeweis $($m.Kennung) fuer $($l.Zeile)",
            "Zeitpunkt:  $(Get-Date -Format o)",
            "Datei:      $($m.Datei.Replace($Wurzel + '\', ''))",
            "Mutation:   $($m.Was)",
            "Fall:       cargo test --manifest-path broker/Cargo.toml $(if ($l.Ziel -eq 'lib') { '--lib' } else { '--test ' + $l.Ziel }) $($l.Test) -- --exact --nocapture",
            "SHA-256 vorher:  $hashVorher",
            "SHA-256 nachher: $hashNachher  (bytegleiche Ruecknahme: $(if ($hashNachher -eq $hashVorher) { 'ja' } else { 'NEIN' }))",
            "Neubau mutiert:      $bauRot",
            "Neubau zurueckgespielt: $bauGruen",
            "Geforderte Rotlaeufe: $($l.Rot); gemessen rot: $($e.Gemessen)",
            "Tragende Meldung (eine davon muss in jedem gemessenen Rotlauf fallen):")) { $inhalt.Add($z) }
        foreach ($erw in $l.Erwartet) { $inhalt.Add("  $erw") }
        $inhalt.Add('')
        $inhalt.Add('--- MUTIERTE STELLE ---')
        foreach ($p in $m.Paare) {
            foreach ($a in $p.Alt) { $inhalt.Add("- $a") }
            foreach ($n in $p.Neu) { $inhalt.Add("+ $n") }
        }
        $inhalt.Add('')
        foreach ($lauf in $e.Laeufe) {
            $inhalt.Add(("--- MUTIERT, Versuch {0} (Frist {1}): Exit {2}, Zusagezeile {3}{4}, gemessen: {5} ---" -f $lauf.Nr, $lauf.Frist, $lauf.Exit, $(if ($lauf.Faellt) { 'faellt' } else { 'faellt NICHT' }), $(if ($l.Freigabegrund) { ", Freigabegrund $($lauf.Grund)" } else { '' }), $(if ($lauf.Gemessen) { 'ja' } else { 'NEIN' })))
            $inhalt.Add($lauf.Text)
            $inhalt.Add('')
        }
        $inhalt.Add(("--- ZURUECKGESPIELT: Exit {0}{1} ---" -f $gruen.ExitCode, $(if ($l.Freigabegrund) { ", Freigabegrund $gruenGrund" } else { '' })))
        $inhalt.Add($gruen.Text)
        $urteil = if (($e.Gemessen -ge $l.Rot) -and $gruenOk -and ($hashNachher -eq $hashVorher)) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }
        $inhalt.Add('')
        $inhalt.Add("URTEIL: $urteil (rot gemessen $($e.Gemessen)/$($l.Rot), gruen Exit $($gruen.ExitCode)$(if ($l.Freigabegrund) { ", Freigabegrund gruen $gruenGrund" }))")
        [System.IO.File]::WriteAllLines((Join-Path $rohOrdner $l.Datei), $inhalt, (New-Object System.Text.UTF8Encoding $false))

        $bilanz.Add([pscustomobject]@{
            Kennung = $m.Kennung; Zeile = $l.Zeile; RotGemessen = "$($e.Gemessen)/$($l.Rot)"
            RotExits = (($e.Laeufe | ForEach-Object { $_.Exit }) -join '/')
            Gruende = (($e.Laeufe | Where-Object { $_.Grund } | ForEach-Object { $_.Grund }) -join '/')
            Gruen = $gruen.ExitCode; GruenGrund = $gruenGrund; HashGleich = ($hashNachher -eq $hashVorher)
            Urteil = $urteil; Datei = $l.Datei })
    }
}

foreach ($d in @($hyp, $inv, $que)) {
    if ((Hash-Von $d) -ne $ausgang[$d]) { throw "Nach dem Lauf weicht $d vom Ausgangsstand ab" }
}

Write-Host ''
Write-Host '=== Bilanz ===' -ForegroundColor Cyan
$bilanz | Format-Table -AutoSize
$bilanzDatei = Join-Path $rohOrdner 'NAK-283-etappe-3-rotbilanz.txt'
$bilanzText = @("NAK-283 Etappe 3 - Bilanz der Rotbeweise ($(Get-Date -Format o))", '')
$bilanzText += @($bilanz | Format-Table -AutoSize | Out-String -Width 400)
$bilanzText += @('Ausgangshashes (vorher = nachher):')
foreach ($d in @($hyp, $inv, $que)) { $bilanzText += ("  {0}  {1}" -f $ausgang[$d], $d.Replace($Wurzel + '\', '')) }
[System.IO.File]::WriteAllLines($bilanzDatei, $bilanzText, (New-Object System.Text.UTF8Encoding $false))
$schlecht = @($bilanz | Where-Object { $_.Urteil -ne 'BESTANDEN' })
if ($schlecht.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($schlecht.Count) Rotbeweis(e)" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Rotbeweise bestanden" -ForegroundColor Green
exit 0
