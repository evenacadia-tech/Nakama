<#
.SYNOPSIS
    S9/SONDE-007b Abschnitt 3 - Installation der Sondenfamilie aus dem
    Installer-Manifest. Beide Haelften: installieren UND Rueckweg.

.DESCRIPTION
    Dieses Skript entscheidet NICHTS. Es liest
    eq-copilot/install/nakama-installer-v1.json (was ausgeliefert wird) und
    eq-copilot/identity/plugin-identities-v1.json (wie die Bundles heissen)
    und tut genau das, was dort steht. Kein Bundlename, kein Zielpfad und
    kein Hash steht in diesem Skript - das war der Fehler der
    Vorgaengerfassung Install-EQ-Copilot.ps1, die drei Hashes und zwei Pfade
    als Literale trug und mit jedem Bau von Hand nachgezogen werden musste.

    Genau EIN Pfadliteral bleibt, und es ist bewusst hier: `Contents\x86_64-win`
    in Innen-Pfad(). Das ist die VST3-Bundlestruktur, kein Nakama-Name - sie
    steht im Format, nicht in unserer Identitaet, und wuerde sich mit einer
    neuen VST3-Fassung aendern, nicht mit einem neuen Bundle. Der Satz oben
    sagt darum "Zielpfad": genannt wird hier nichts, was in
    plugin-identities-v1.json oder im Installer-Manifest steht.
    (T1-Selbstaudit 23.08., Punkt "luegt der Text?" - vorher stand da
    pauschal "kein Pfad", und das war um dieses Fragment zu viel.)

    DREI RIEGEL, VOR JEDEM SCHREIBZUGRIFF:
      1. Vollstaendigkeit - ein Artefakt mit `sha256: null` bricht ab. Ein
         halb eingefrorenes Paket ist kein Paket.
      2. Echtheit - jede Quelldatei wird gegen ihren Manifest-Hash geprueft,
         BEVOR irgendetwas kopiert wird. Und danach noch einmal am Ziel.
      3. Ruhe - laeuft FL Studio, wird nichts angefasst. Ein Host, der ein
         Bundle geladen haelt, bekommt sonst eine Datei unter den Fingern
         weggetauscht.

    GEGENPFAD (Hausinvariante: installieren <-> Rollback im selben
    Aenderungssatz): vor jedem Tausch wandert der VORGEFUNDENE Stand in den
    geschuetzten Transaktionsordner unter Program Files. Nur dessen atomarer
    Anker autorisiert `-Rueckweg`; install/install-ergebnis.json ist reine
    Diagnose. Der Rueckweg verweigert, wenn er NAK-41 ausloesen wuerde
    (Rueckfall auf einen Build mit kleinerem State-Schema: das Projekt
    verliert dann still seine Messpunkt-Identitaet).

    DIE AUSLIEFERUNGSEINHEIT IST DER BUNDLE-ORDNER (S9-Nacharbeit 23.08.2026,
    T2-Befund T2-5; Vertrag §2.1). Ein VST3-Bundle ist unter Windows ein
    Ordner: neben der Binaerdatei liegt Contents/Resources/moduleinfo.json.
    Bis dahin adressierte dieses Skript nur die innere Binaerdatei - eine
    Erstinstallation haette ein Bundle OHNE moduleinfo.json erzeugt, beim
    Main-Bundle waere das vom 16.08. ("Version": "0.1.0") neben einer
    0.3.0-Binaerdatei liegengeblieben, `-Pruefen` haette dazu `aktuell`
    gesagt, und der Rueckweg haette ein leeres .vst3-Gehaeuse stehenlassen.
    Ordner also ueberall: hashen, sichern, tauschen, pruefen, zurueck.

    Der Ordner-Hash selbst steht in NakamaOrdnerHash.ps1 - EINE Funktion,
    zwei Aufrufer (dieses Skript und die Kreuzprobe des Kanon-Beins A17,
    die ihn gegen die Python-Haelfte misst).

.PARAMETER Pruefen
    Nichts schreiben. Nur berichten, was passieren wuerde und ob der
    installierte Stand mit dem Manifest uebereinstimmt (Repair-Diagnose).

.PARAMETER Rueckweg
    Den vor der letzten Installation gesicherten Stand wiederherstellen.

.PARAMETER Erzwingen
    Nur mit -Rueckweg: den NAK-41-Riegel uebergehen. Sichtbar, nie still.

.NOTES
    Braucht Rechteerhoehung (Program Files). Die Installation bleibt ein
    Klick des Users - dieses Skript wird von keiner Automatik gefahren
    (NAK-32).
#>

[CmdletBinding()]
param(
    [switch]$Pruefen,
    [switch]$Rueckweg,
    [switch]$Erzwingen
)

$ErrorActionPreference = 'Stop'

# ── Wurzel und Verträge ────────────────────────────────────────────────────
# $PSScriptRoot ist eq-copilot/install; die Manifest-Pfade sind relativ zur
# Repo-Wurzel, damit Skript und Pruefbein dieselbe Zeile gleich lesen.
$wurzel      = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$manifestWeg = Join-Path $PSScriptRoot 'nakama-installer-v1.json'
$ergebnisWeg = Join-Path $PSScriptRoot 'install-ergebnis.json'
$journalSchema = 'nakama.install-ergebnis/v1'
$script:transaktionsWeg = $null

# Ordner-Hash v1 (Vertrag §2.1). Dot-Source, weil dieselbe Funktion auch die
# Kreuzprobe von A17 ruft: waere sie hier eingebettet, koennte die Kreuzprobe
# nur eine Kopie messen, und zwei Kopien laufen auseinander.
$ordnerHashWeg = Join-Path $PSScriptRoot 'NakamaOrdnerHash.ps1'
if (-not (Test-Path -LiteralPath $ordnerHashWeg)) {
    Write-Host "ABBRUCH: NakamaOrdnerHash.ps1 fehlt neben diesem Skript ($ordnerHashWeg)." -ForegroundColor Red
    exit 1
}
. $ordnerHashWeg

function Kanonischer-Pfad([string]$pfad) {
    if ([string]::IsNullOrWhiteSpace($pfad)) { throw 'Leerer Pfad ist kein Pfad.' }
    return [IO.Path]::GetFullPath($pfad)
}

function Ist-GleichOderUnter([string]$pfad, [string]$basis) {
    $p = Kanonischer-Pfad $pfad
    $b = (Kanonischer-Pfad $basis).TrimEnd('\', '/')
    return $p.Equals($b, [StringComparison]::OrdinalIgnoreCase) -or
           $p.StartsWith($b + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)
}

function Fordere-GleichOderUnter([string]$pfad, [string]$basis, [string]$was) {
    if (-not (Ist-GleichOderUnter $pfad $basis)) {
        Abbruch "$was verlaesst die erlaubte Wurzel '$basis': $pfad"
    }
}

function Fordere-Keine-Reparsepunkte([string]$pfad, [string]$was) {
    $lauf = Kanonischer-Pfad $pfad
    while (-not [string]::IsNullOrEmpty($lauf)) {
        if (Test-Path -LiteralPath $lauf) {
            $item = Get-Item -LiteralPath $lauf -Force
            if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                Abbruch "$was enthaelt einen Reparsepunkt/Junction und wird nicht erhoeht verfolgt: $lauf"
            }
        }
        $eltern = Split-Path -Parent $lauf
        if ([string]::IsNullOrEmpty($eltern) -or $eltern -eq $lauf) { break }
        $lauf = $eltern
    }
}

function Loese-Pfad([string]$relativ) {
    if ([IO.Path]::IsPathRooted($relativ)) { Abbruch "Repo-Pfad muss relativ sein: $relativ" }
    $voll = Kanonischer-Pfad (Join-Path $wurzel ($relativ -replace '/', '\'))
    Fordere-GleichOderUnter $voll $wurzel 'Repo-Pfad'
    return $voll
}

function Schreibe-JsonAtomar([string]$weg, $stand) {
    $tmp = $weg + '.tmp-' + [Guid]::NewGuid().ToString('N')
    $strom = $null
    try {
        # CreateNew macht auch die Guid nicht nur wahrscheinlich, sondern
        # tatsaechlich exklusiv. UTF-8 ohne BOM ist fuer Python und PowerShell
        # derselbe Vertrag; Flush(true) schliesst das Hard-Crash-Fenster vor
        # der atomaren Veroeffentlichung.
        $bytes = [Text.UTF8Encoding]::new($false).GetBytes(
            (($stand | ConvertTo-Json -Depth 10) + [Environment]::NewLine))
        $strom = [IO.FileStream]::new(
            $tmp, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write,
            [IO.FileShare]::None, 4096, [IO.FileOptions]::WriteThrough)
        $strom.Write($bytes, 0, $bytes.Length)
        $strom.Flush($true)
        $strom.Dispose()
        $strom = $null

        if (Test-Path -LiteralPath $weg -PathType Leaf) {
            $keinBackup = [System.Management.Automation.Language.NullString]::Value
            [IO.File]::Replace($tmp, $weg, $keinBackup, $true)
        }
        else {
            try { [IO.File]::Move($tmp, $weg) }
            catch [IO.IOException] {
                # Falls das Ziel im schmalen Rennen doch erschien, bleibt die
                # zweite Veroeffentlichung ebenfalls ein atomarer Replace.
                if (-not (Test-Path -LiteralPath $weg -PathType Leaf)) { throw }
                $keinBackup = [System.Management.Automation.Language.NullString]::Value
                [IO.File]::Replace($tmp, $weg, $keinBackup, $true)
            }
        }
    }
    finally {
        if ($null -ne $strom) { $strom.Dispose() }
        if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force }
    }
}

function Schreibe-Ergebnis($stand) {
    # Die Kopie im Benutzer-Repo ist Diagnose, nicht Autoritaet. Fuer jeden
    # mutationsfaehigen Lauf wird ZUERST der geschuetzte Spiegel unter dem
    # fest verdrahteten Program-Files-Baum veroeffentlicht. Ein Crash zwischen
    # beiden Writes laesst damit hoechstens die Diagnose altern, nie den
    # Gegenpfad.
    if (-not [string]::IsNullOrWhiteSpace([string]$script:transaktionsWeg)) {
        Schreibe-JsonAtomar $script:transaktionsWeg $stand
    }
    Schreibe-JsonAtomar $ergebnisWeg $stand
}

function Abbruch([string]$grund) {
    Write-Host "ABBRUCH: $grund" -ForegroundColor Red
    # Ein -Pruefen-Lauf schreibt NICHTS, auch kein Fehlerprotokoll: sonst
    # ueberschriebe eine Diagnose den Rueckweg-Stand der letzten echten
    # Installation, und der Gegenpfad haette keine Quelle mehr.
    #
    # DASSELBE GILT FUER EINEN GESCHEITERTEN RUECKWEG - und das war bis zur
    # S9-Nacharbeit (23.08.2026) nicht so. Gefunden beim GEFAHRENEN
    # Gegenpfad: der NAK-41-Riegel verweigert und sagt "Mit -Erzwingen
    # ausdruecklich wiederholen" - schrieb dabei aber `status: ERROR` in
    # install-ergebnis.json und zerstoerte damit genau die Quelle, aus der
    # die Wiederholung haette lesen muessen. Der zweite Lauf brach mit
    # "Der letzte Lauf endete mit 'ERROR' - nichts zu sichern" ab: der
    # Ausweg, den der Riegel anbietet, war nach EINER Verweigerung zu.
    # Ein Riegel, der den eigenen Ausweg zumauert, ist kein Riegel, sondern
    # eine Sackgasse - und die Zeile darueber war damit ein luegender Text.
    exit 1
}

if (-not (Test-Path -LiteralPath $manifestWeg)) { Abbruch "Installer-Manifest fehlt: $manifestWeg" }

$manifest   = Get-Content -LiteralPath $manifestWeg -Raw | ConvertFrom-Json
$identWeg   = Loese-Pfad $manifest.identitaetsquelle
if (-not (Test-Path -LiteralPath $identWeg)) { Abbruch "Identitaetsdatei fehlt: $identWeg" }
$identitaet = Get-Content -LiteralPath $identWeg -Raw | ConvertFrom-Json

if ($manifest.schema -ne 'nakama.installer/v1') {
    Abbruch "Fremdes Manifestschema '$($manifest.schema)' - dieses Skript liest nur nakama.installer/v1."
}
if ($identitaet.schema -ne 'nakama.plugin-identities/v1') {
    Abbruch "Fremdes Identitaetsschema '$($identitaet.schema)'."
}
if (-not [IO.Path]::IsPathRooted([string]$manifest.ziele.vst3_verzeichnis) -or
    -not [IO.Path]::IsPathRooted([string]$manifest.ziele.broker_verzeichnis)) {
    Abbruch 'Beide Installationsziele muessen absolute Pfade sein.'
}
$vst3Basis = Kanonischer-Pfad ($manifest.ziele.vst3_verzeichnis -replace '/', '\')
$brokerBasis = Kanonischer-Pfad ($manifest.ziele.broker_verzeichnis -replace '/', '\')
$erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
$erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
if (-not $vst3Basis.Equals($erlaubteVst3Basis, [StringComparison]::OrdinalIgnoreCase) -or
    -not $brokerBasis.Equals($erlaubteBrokerBasis, [StringComparison]::OrdinalIgnoreCase)) {
    Abbruch "Installationsziele weichen von der kanonischen Produktpolicy ab: VST3 '$erlaubteVst3Basis', Broker '$erlaubteBrokerBasis'."
}
$transaktionsOrdner = Kanonischer-Pfad (Join-Path (Split-Path -Parent $erlaubteBrokerBasis) '.nakama-installer')
$script:transaktionsWeg = Kanonischer-Pfad (Join-Path $transaktionsOrdner 'aktive-transaktion.json')
$sicherungsOrdner = Kanonischer-Pfad (Join-Path $transaktionsOrdner 'backups')
$script:geschuetzterInstallerLock = $null
foreach ($basis in @($vst3Basis, $brokerBasis)) {
    $laufwerksWurzel = [IO.Path]::GetPathRoot($basis).TrimEnd('\', '/')
    if ($basis.TrimEnd('\', '/').Equals($laufwerksWurzel, [StringComparison]::OrdinalIgnoreCase)) {
        Abbruch "Ein Installationsziel darf keine Laufwerkswurzel sein: $basis"
    }
}
if ((Ist-GleichOderUnter $vst3Basis $brokerBasis) -or
    (Ist-GleichOderUnter $brokerBasis $vst3Basis)) {
    Abbruch 'VST3- und Broker-Zielverzeichnis duerfen weder gleich noch ineinander liegen.'
}

# Ziel-ID -> Identitaetseintrag. Der Bundlename kommt AUSSCHLIESSLICH von hier.
$zieleNachId = @{}
$bundleNamen = @{}
foreach ($z in $identitaet.ziele) {
    if ([string]::IsNullOrWhiteSpace([string]$z.id) -or $zieleNachId.ContainsKey([string]$z.id)) {
        Abbruch "Identitaets-ID fehlt oder ist doppelt: '$($z.id)'."
    }
    if ([string]::IsNullOrWhiteSpace([string]$z.bundle) -or
        [IO.Path]::GetFileName([string]$z.bundle) -ne [string]$z.bundle) {
        Abbruch "Bundle ist kein einzelner sicherer Dateiname: '$($z.bundle)'."
    }
    $bundleKey = ([string]$z.bundle).ToUpperInvariant()
    if ($bundleNamen.ContainsKey($bundleKey)) { Abbruch "Bundle-Ziel ist doppelt: '$($z.bundle)'." }
    if (($z.state_schema -isnot [int] -and $z.state_schema -isnot [long]) -or [int64]$z.state_schema -ne 2) {
        Abbruch "state_schema fuer '$($z.id)' muss typstreng dem aktuellen NakamaState-Schema 2 entsprechen."
    }
    $zieleNachId[[string]$z.id] = $z
    $bundleNamen[$bundleKey] = $true
}

<# Nachschlagen mit LEEREM Schluessel ist eine Frage, keine Ausnahme.
   PowerShell wirft bei `$hashtable[$null]` ("Index operation failed; the
   array index evaluated to null") - und der Broker-Eintrag traegt
   berechtigterweise keine ziel_id. Gefunden beim GEFAHRENEN Gegenpfad
   (S9-Nacharbeit 23.08.2026): der Rueckweg starb genau dann, wenn der
   Broker schon einmal installiert war, also ab der zweiten Installation.
   Reines Lesen hatte das nicht gezeigt. #>
function Ident-Von($zielId) {
    if ([string]::IsNullOrEmpty($zielId)) { return $null }
    return $zieleNachId[$zielId]
}

function Bekannt-Von($tabelle, $schluessel) {
    if ([string]::IsNullOrEmpty($schluessel)) { return $null }
    return $tabelle[$schluessel]
}

# `art` ist eine GESCHLOSSENE Menge (Vertrag §2.2, T2-Befund T2-4). Bis zur
# S9-Nacharbeit behandelte Ziel-Pfad() alles, was nicht 'vst3' ist, im
# Broker-Zweig - ein Tippfehler beim Erweitern legte die Datei also unter
# ziele.broker_verzeichnis, in genau den Pfad, den §4 schuetzt, weil er ab
# SONDE-010 ein Spawn-Ziel wird. Der Riegel steht sowohl im Kanon-Bein A17
# (misst das COMMITTETE Manifest) als auch hier (misst das Manifest, das
# GERADE ausgefuehrt wird) - zwei verschiedene Fragen, keine ersetzt die andere.
foreach ($a in $manifest.artefakte) {
    if ($a.art -notin @('vst3', 'broker')) {
        Abbruch "Unbekannte Artefaktsorte '$($a.art)'. Der Vertrag kennt genau zwei: vst3 und broker. Ein dritter Wert landete sonst im Broker-Zweig - also im geschuetzten Spawn-Pfad."
    }
}
# S9b/SONDE-007c (28.08.2026): stillgelegte Ziele gehoeren NICHT in die
# Auslieferung - und die Sollmenge ist ab hier "jedes Ziel OHNE das Feld
# `stillgelegt`". Wichtig ist die zweite Haelfte darunter: ein stillgelegtes
# Ziel muss im Manifest ausdruecklich BENANNT sein. Ohne sie koennte ein Ziel
# still aus der Auslieferung fallen, indem jemand nur seinen Artefakteintrag
# loescht - der Zaehlvergleich bliebe gruen, weil beide Seiten schrumpfen.
# Nacharbeit Runde 1 (29.08.2026, T2-Befund P1 Nr. 1): die Trennung faellt auf
# die ANWESENHEIT der Marke, nicht auf ihren Inhalt. Hier stand
# `$null -ne $_.stillgelegt`, und gemessen (Manifest SONDE-007c C2d) liefert
# eine Property mit JSON-`null` denselben `$null` wie eine FEHLENDE Property:
# ein Ziel mit `"stillgelegt": null` zaehlte als AKTIV, und der Lauf brach mit
# einer Meldung ab, die das falsche Problem benannte ("Manifest muss jedes
# NICHT stillgelegte Identitaetsziel ... enthalten"). Und in einer
# auslieferungsreifen Sandbox (Bein A18 [3d], gegen die Fassung vor dieser
# Runde gefahren) lief eine Marke vom Typ String, Array oder Zahl mit Exit 0
# durch - eine unlesbare Stilllegung, ueber die niemand etwas sagte.
# Die Anwesenheit fragt in PowerShell nur `PSObject.Properties` ab; ein
# kaputter Inhalt ist danach ein harter Fehler, nie ein Ruecksprung nach
# "aktiv". Gefahren wird der Fall im Kanon-Bein A18, Block [3d].
foreach ($z in @($identitaet.ziele | Where-Object { $null -ne $_ })) {
    if (($z.PSObject.Properties.Name -contains 'stillgelegt') -and
        ($z.stillgelegt -isnot [System.Management.Automation.PSCustomObject])) {
        Abbruch "Ziel '$($z.id)' traegt eine unlesbare Stilllegungsmarke - erwartet wird ein Objekt mit Datum und Entscheid. Eine kaputte Marke bedeutet weiter 'stillgelegt': repariere sie in der Identitaetsdatei, entferne sie nicht."
    }
}
$stillgelegteIds = @($identitaet.ziele | Where-Object { $_.PSObject.Properties.Name -contains 'stillgelegt' } | ForEach-Object { [string]$_.id })
$vst3Ids = @($manifest.artefakte | Where-Object { $_.art -eq 'vst3' } | ForEach-Object { [string]$_.ziel_id })
$sollIds = @($identitaet.ziele | Where-Object { $_.PSObject.Properties.Name -notcontains 'stillgelegt' } | ForEach-Object { [string]$_.id })
if ($sollIds.Count -eq 0) {
    Abbruch 'Kein einziges aktives Identitaetsziel - eine Auslieferung ohne Bundle waere keine.'
}
if ($vst3Ids.Count -ne $sollIds.Count -or
    @($vst3Ids | Sort-Object -Unique).Count -ne $vst3Ids.Count -or
    (Compare-Object ($vst3Ids | Sort-Object) ($sollIds | Sort-Object))) {
    Abbruch 'Das Manifest muss jedes NICHT stillgelegte Identitaetsziel genau einmal als VST3-Artefakt enthalten.'
}
$benannteStillgelegte = @($manifest.stillgelegte_ziele | Where-Object { $null -ne $_ } | ForEach-Object { [string]$_.ziel_id })
# Der Zaehlvergleich steht VOR dem Mengenvergleich, und der Mengenvergleich
# laeuft nur bei nicht-leeren Listen. Gemessen (T1-Selbstaudit 28.08.2026):
# `@() | Sort-Object` liefert $null, und `Compare-Object $null $null` bricht
# mit "Cannot bind argument to parameter 'ReferenceObject'" ab - unter
# $ErrorActionPreference='Stop' also ein harter Fehler statt eines gruenen
# Durchlaufs. Heute feuert das nicht, weil es genau ein stillgelegtes Ziel
# gibt; es feuerte an dem Tag, an dem das letzte wieder verschwindet.
if ($benannteStillgelegte.Count -ne $stillgelegteIds.Count -or
    @($benannteStillgelegte | Sort-Object -Unique).Count -ne $benannteStillgelegte.Count -or
    ($benannteStillgelegte.Count -gt 0 -and
     (Compare-Object ($benannteStillgelegte | Sort-Object) ($stillgelegteIds | Sort-Object)))) {
    Abbruch 'Jedes stillgelegte Identitaetsziel muss im Manifest unter `stillgelegte_ziele` genau einmal benannt sein - und nur die stillgelegten.'
}
$brokerArtefakte = @($manifest.artefakte | Where-Object { $_.art -eq 'broker' })
if ($brokerArtefakte.Count -ne 1) { Abbruch 'Das Manifest muss genau ein Broker-Artefakt enthalten.' }

<# S9b/SONDE-007c (28.08.2026): ein stillgelegtes Ziel, das auf dieser
   Maschine noch installiert liegt.

   ENTSCHEIDUNG: melden, nicht loeschen. Der Grund steht im Manifest
   (`stillgelegte_ziele[].umgang_begruendung`) und ist zweiteilig: der
   Gegenpfad installieren<->Rueckweg ist eine strenge 1:1-Beziehung zwischen
   Journal und `artefakte` - eine Loeschung waere ein vierter Akt, dessen
   Rueckweg das Bundle wiederherstellen muesste, sonst verloere der Rueckweg
   einen Stand statt ihn zurueckzugeben. Und Common Files\VST3 gehoert nicht
   diesem Installer.

   Was NICHT passiert: stillschweigen. Die Meldung laeuft im normalen
   Installationslauf UND in -Pruefen, mit vollem Pfad und dem Handgriff. #>
function Melde-StillgelegteAltlasten {
    foreach ($s in @($manifest.stillgelegte_ziele | Where-Object { $null -ne $_ })) {
        $ident = Ident-Von ([string]$s.ziel_id)
        if ($null -eq $ident) {
            Abbruch "Stillgelegtes Ziel '$($s.ziel_id)' steht nicht in der Identitaetsdatei - die Kennung darf nicht geloescht werden."
        }
        # Derselbe Weg wie fuer ein aktives Ziel: Bundlename AUSSCHLIESSLICH
        # aus der Identitaetsdatei, Pfad gegen das Zielverzeichnis gerichtet.
        if ([IO.Path]::GetFileName($ident.bundle) -ne $ident.bundle) {
            Abbruch "Bundle '$($ident.bundle)' ist kein einzelner sicherer Dateiname."
        }
        $alt = Kanonischer-Pfad (Join-Path $vst3Basis $ident.bundle)
        if (-not (Kanonischer-Pfad (Split-Path -Parent $alt)).Equals($vst3Basis, [StringComparison]::OrdinalIgnoreCase)) {
            Abbruch "Pfad eines stillgelegten Ziels verlaesst sein Zielverzeichnis: $alt"
        }
        if (Test-Path -LiteralPath $alt) {
            Write-Host ''
            Write-Host "ALTLAST: '$($ident.produktname)' ist seit $($s.seit) stillgelegt, liegt hier aber noch installiert." -ForegroundColor Yellow
            Write-Host "  $alt" -ForegroundColor Yellow
            Write-Host "  Dieser Installer entfernt es NICHT ($($s.umgang_mit_altbestand)). Von Hand, mit Adminrechten und geschlossenem FL:" -ForegroundColor Yellow
            Write-Host "    Remove-Item -Recurse -Force '$alt'" -ForegroundColor Yellow
            Write-Host "  Grund der Stilllegung: $($s.warum)" -ForegroundColor DarkGray
            Write-Host ''
        }
        else {
            Write-Host "  ok      $($ident.produktname) : stillgelegt seit $($s.seit), nicht installiert"
        }
    }
}

function Ziel-Pfad($artefakt) {
    if ($artefakt.art -eq 'vst3') {
        $ident = Ident-Von $artefakt.ziel_id
        if ($null -eq $ident) { Abbruch "Ziel '$($artefakt.ziel_id)' steht nicht in der Identitaetsdatei." }
        # Der BUNDLE-ORDNER, nicht die Datei darin (Vertrag §2.1).
        if ([IO.Path]::GetFileName($ident.bundle) -ne $ident.bundle) {
            Abbruch "Bundle '$($ident.bundle)' ist kein einzelner sicherer Dateiname."
        }
        $basis = Kanonischer-Pfad ($manifest.ziele.vst3_verzeichnis -replace '/', '\')
        $ziel = Kanonischer-Pfad (Join-Path $basis $ident.bundle)
        if (-not (Kanonischer-Pfad (Split-Path -Parent $ziel)).Equals($basis, [StringComparison]::OrdinalIgnoreCase)) {
            Abbruch "VST3-Ziel verlaesst sein Zielverzeichnis: $ziel"
        }
        return $ziel
    }
    if ([IO.Path]::GetFileName($artefakt.name) -ne $artefakt.name) {
        Abbruch "Brokername '$($artefakt.name)' ist kein einzelner sicherer Dateiname."
    }
    $basis = Kanonischer-Pfad ($manifest.ziele.broker_verzeichnis -replace '/', '\')
    $ziel = Kanonischer-Pfad (Join-Path $basis $artefakt.name)
    if (-not (Kanonischer-Pfad (Split-Path -Parent $ziel)).Equals($basis, [StringComparison]::OrdinalIgnoreCase)) {
        Abbruch "Brokerziel verlaesst sein Zielverzeichnis: $ziel"
    }
    return $ziel
}

<# Die innere Binaerdatei eines Bundles. Nur noch fuer EINEN Zweck da: die
   zwei historischen `bekannte_staende`-Eintraege wurden gegen sie
   eingefroren, lange bevor der Ordner die Einheit war (Vertrag §5.1). #>
function Innen-Pfad($artefakt, [string]$bundleOrdner) {
    if ($artefakt.art -ne 'vst3') { return $null }
    $ident = Ident-Von $artefakt.ziel_id
    if ($null -eq $ident) { return $null }
    return Join-Path $bundleOrdner (Join-Path 'Contents\x86_64-win' $ident.bundle)
}

function Artefakt-Name($artefakt) {
    if ($artefakt.art -eq 'vst3') {
        $ident = Ident-Von $artefakt.ziel_id
        # Diese Funktion laeuft VOR Ziel-Pfad (im Vollstaendigkeitsriegel).
        # Ohne den Zweig verungluecktete ein vst3-Eintrag ohne ziel_id hier
        # mit einem Indexfehler statt mit einem Satz, den jemand lesen kann.
        if ($null -eq $ident) { return "<vst3 ohne ziel_id: '$($artefakt.ziel_id)'>" }
        return $ident.produktname
    }
    return $artefakt.name
}

<# Der Hash EINES Artefakts, art-abhaengig: `vst3` ist ein Ordner (Ordner-Hash
   v1), `broker` eine Datei. $null heisst "liegt nicht vor" - das ist eine
   Antwort, kein Fehler. #>
function Hash-Von([string]$pfad, [string]$art) {
    return Get-NakamaArtefaktHash -Pfad $pfad -Art $art
}

function Datei-Hash([string]$pfad) {
    if (-not (Test-Path -LiteralPath $pfad -PathType Leaf)) { return $null }
    return (Get-FileHash -LiteralPath $pfad -Algorithm SHA256).Hash
}

<# Legt $pfad an und liefert die Liste der TATSAECHLICH neu erzeugten
   Verzeichnisse (von aussen nach innen). Nur diese darf der Rueckweg wieder
   entfernen - ein vorgefundenes Verzeichnis gehoert uns nicht. #>
function Erzeuge-Ordnerkette([string]$pfad) {
    if (Test-Path -LiteralPath $pfad) { return @() }
    # Nur die konfigurierte Zielwurzel gehoert ins Loeschjournal. New-Item legt
    # noetige Eltern zwar mit an; deren Besitz ist aber nicht beweisbar.
    New-Item -ItemType Directory -Path $pfad -Force | Out-Null
    return @((Kanonischer-Pfad $pfad))
}

<# Entfernt einen Stand vollstaendig - beim Bundle den ganzen ORDNER. Eine
   Datei darin zu loeschen und das Gehaeuse stehenzulassen war genau der
   Befund T2-5 (d). #>
function Entferne-Stand([string]$pfad, [string]$art) {
    if (-not (Test-Path -LiteralPath $pfad)) { return $false }
    if ($art -eq 'vst3') { Remove-Item -LiteralPath $pfad -Recurse -Force }
    else                 { Remove-Item -LiteralPath $pfad -Force }
    return $true
}

<# Kopiert einen Stand. Das Ziel darf NICHT existieren: `Copy-Item -Recurse`
   auf einen vorhandenen Ordner FUEHRT ZUSAMMEN, statt zu ersetzen - eine
   Datei, die der neue Bau nicht mehr enthaelt, ueberlebte still im
   installierten Bundle (Befund T2-5 (b)). #>
function Kopiere-Stand([string]$von, [string]$nach, [string]$art) {
    if (Test-Path -LiteralPath $nach) { Entferne-Stand $nach $art | Out-Null }
    if ($art -eq 'vst3') { Copy-Item -LiteralPath $von -Destination $nach -Recurse -Force }
    else                 { Copy-Item -LiteralPath $von -Destination $nach -Force }
}

<# Die Sicherung und ihr Erwartungshash liegen beide unter der geschuetzten
   Transaktionswurzel. Vor dem Restore wird sie trotzdem in einen eindeutigen
   Staging-Pfad kopiert und erneut gehasht: so gelangt auch bei einem
   unerwarteten lokalen Dateisystemrennen kein ungepruefter Stand ins Ziel. #>
function Stelle-Gepruefte-SicherungWiederHer(
    [string]$sicherung, [string]$ziel, [string]$art, [string]$erwarteterHash) {
    $endung = if ($art -eq 'vst3') { '.bundle' } else { '.bak' }
    $staging = Kanonischer-Pfad (Join-Path $transaktionsOrdner ('restore-' + [Guid]::NewGuid().ToString('N') + $endung))
    if (-not (Ist-GleichOderUnter $staging $transaktionsOrdner)) {
        throw "Restore-Staging verlaesst den geschuetzten Transaktionsordner: $staging"
    }
    try {
        Kopiere-Stand $sicherung $staging $art
        if ((Hash-Von $staging $art) -ne $erwarteterHash) {
            throw "Sicherung stimmt beim geschuetzten Staging nicht mehr: $sicherung"
        }
        Kopiere-Stand $staging $ziel $art
        if ((Hash-Von $ziel $art) -ne $erwarteterHash) {
            throw "Wiederhergestelltes Ziel stimmt nicht mit dem Vorzustand ueberein: $ziel"
        }
    }
    finally {
        if (Test-Path -LiteralPath $staging) { Entferne-Stand $staging $art | Out-Null }
    }
}

function Fl-Laeuft {
    $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
}

function Ist-Sha256($wert) {
    return $null -ne $wert -and ([string]$wert) -match '^[0-9A-Fa-f]{64}$'
}

function Ist-TransaktionsId($wert) {
    return $null -ne $wert -and ([string]$wert) -match '^[0-9a-f]{32}$'
}

function Sicherungsordner-Fuer([string]$transaktionsId) {
    if (-not (Ist-TransaktionsId $transaktionsId)) {
        Abbruch "Ungueltige Transaktions-ID '$transaktionsId'."
    }
    $ordner = Kanonischer-Pfad (Join-Path $sicherungsOrdner $transaktionsId)
    if (-not (Kanonischer-Pfad (Split-Path -Parent $ordner)).Equals(
            $sicherungsOrdner, [StringComparison]::OrdinalIgnoreCase)) {
        Abbruch "Transaktionssicherung verlaesst die geschuetzte Sicherungswurzel: $ordner"
    }
    return $ordner
}

function Loese-GeschuetzteSicherung([string]$relativ, [string]$transaktionsId) {
    if ([string]::IsNullOrWhiteSpace($relativ) -or [IO.Path]::IsPathRooted($relativ)) {
        Abbruch "Sicherung muss relativ zur geschuetzten Transaktionswurzel sein: '$relativ'"
    }
    $txOrdner = Sicherungsordner-Fuer $transaktionsId
    $voll = Kanonischer-Pfad (Join-Path $transaktionsOrdner ($relativ -replace '/', '\'))
    if (-not (Kanonischer-Pfad (Split-Path -Parent $voll)).Equals(
            $txOrdner, [StringComparison]::OrdinalIgnoreCase)) {
        Abbruch "Sicherung verlaesst ihren geschuetzten Transaktionsordner: $voll"
    }
    return $voll
}

function Entferne-TransaktionsSicherungen([string]$transaktionsId) {
    if (-not (Ist-TransaktionsId $transaktionsId)) { return }
    $ordner = Sicherungsordner-Fuer $transaktionsId
    if (-not (Test-Path -LiteralPath $ordner)) { return }
    Fordere-GleichOderUnter $ordner $sicherungsOrdner 'zu entfernende Transaktionssicherung'
    Fordere-Keine-Reparsepunkte $ordner 'zu entfernende Transaktionssicherung'
    Remove-Item -LiteralPath $ordner -Recurse -Force
}

function Bereinige-VerwaisteSicherungen([string]$behaltenId) {
    if (-not (Test-Path -LiteralPath $sicherungsOrdner -PathType Container)) { return }
    Fordere-Keine-Reparsepunkte $sicherungsOrdner 'geschuetzte Sicherungswurzel'
    foreach ($ordner in @(Get-ChildItem -LiteralPath $sicherungsOrdner -Directory -Force)) {
        $id = $ordner.Name
        if (-not (Ist-TransaktionsId $id) -or $id -eq $behaltenId) { continue }
        Entferne-TransaktionsSicherungen $id
    }
}

function Entferne-LeereTransaktionsWurzeln {
    if ((Test-Path -LiteralPath $sicherungsOrdner -PathType Container) -and
        @(Get-ChildItem -LiteralPath $sicherungsOrdner -Force).Count -eq 0) {
        Remove-Item -LiteralPath $sicherungsOrdner -Force
    }
    if ((Test-Path -LiteralPath $transaktionsOrdner -PathType Container) -and
        @(Get-ChildItem -LiteralPath $transaktionsOrdner -Force).Count -eq 0) {
        Remove-Item -LiteralPath $transaktionsOrdner -Force
    }
}

function Sperre-GeschuetztenInstaller {
    Fordere-Keine-Reparsepunkte $transaktionsOrdner 'geschuetzte Transaktionswurzel'
    if (-not (Test-Path -LiteralPath $transaktionsOrdner)) {
        New-Item -ItemType Directory -Path $transaktionsOrdner -Force | Out-Null
    }
    Fordere-Keine-Reparsepunkte $transaktionsOrdner 'geschuetzte Transaktionswurzel'
    $lockWeg = Kanonischer-Pfad (Join-Path $transaktionsOrdner 'installer.lock')
    try {
        $script:geschuetzterInstallerLock = [IO.FileStream]::new(
            $lockWeg, [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite,
            [IO.FileShare]::None, 1, [IO.FileOptions]::DeleteOnClose)
    }
    catch {
        Abbruch "Ein anderer mutationsfaehiger Nakama-Installerlauf ist aktiv ($lockWeg)."
    }
}

# Alle aufgeloesten Ziele muessen eindeutig sein. Das gilt auch im Rueckweg,
# bevor eine fremde Ergebnisdatei ueberhaupt betrachtet wird.
$aufgeloesteZiele = @{}
foreach ($a in $manifest.artefakte) {
    $ziel = Ziel-Pfad $a
    $key = $ziel.ToUpperInvariant()
    if ($aufgeloesteZiele.ContainsKey($key)) { Abbruch "Installationsziel ist doppelt: $ziel" }
    $aufgeloesteZiele[$key] = $true
    Fordere-Keine-Reparsepunkte $ziel 'Installationsziel'
}

# ═══════════════════════════════════════════════════ RUECKWEG ══════════════
if ($Rueckweg) {
    if (-not (Test-Path -LiteralPath $transaktionsWeg -PathType Leaf)) {
        Abbruch "Kein geschuetztes Transaktionsjournal ($transaktionsWeg) - die Repo-Diagnose allein autorisiert keinen Rueckweg."
    }
    Sperre-GeschuetztenInstaller
    Fordere-Keine-Reparsepunkte $transaktionsWeg 'geschuetztes Transaktionsjournal'
    try { $letzte = Get-Content -LiteralPath $transaktionsWeg -Raw | ConvertFrom-Json }
    catch { Abbruch "Geschuetztes Transaktionsjournal ist kein lesbares JSON: $($_.Exception.Message)" }
    $rueckwegFaehigeStaende = @('OK', 'VORBEREITET', 'KOMPENSATION', 'ERROR_TEILSTAND', 'RUECKWEG_AKTIV')
    if ($letzte.schema -ne $journalSchema -or $letzte.status -notin $rueckwegFaehigeStaende) {
        Abbruch "Der letzte Lauf ist kein rueckwegfaehiges $journalSchema."
    }
    if (-not (Ist-TransaktionsId $letzte.transaktions_id)) {
        Abbruch 'Geschuetztes Transaktionsjournal hat keine gueltige Transaktions-ID.'
    }
    $aktiveTransaktionsId = [string]$letzte.transaktions_id
    $aktiveSicherungen = Sicherungsordner-Fuer $aktiveTransaktionsId
    if (Fl-Laeuft) { Abbruch 'FL Studio laeuft. Der Rueckweg fasst nichts an.' }

    # Das Benutzer-Repo liefert nur Daten, niemals privilegierte Pfade. Jeder
    # Journaleintrag wird vor der ersten Mutation aus Manifest+Identitaet neu
    # berechnet; Sicherungen bleiben im fest gebundenen geschuetzten
    # Unterordner genau dieser Transaktion.
    if (@($letzte.eintraege).Count -ne @($manifest.artefakte).Count) {
        Abbruch 'Rueckwegjournal und Manifest haben verschieden viele Artefakte.'
    }
    $validiert = @()
    $erzeugteGesehen = @{}
    for ($i = 0; $i -lt @($manifest.artefakte).Count; ++$i) {
        $a = @($manifest.artefakte)[$i]
        $e = @($letzte.eintraege)[$i]
        foreach ($flagName in @('mutation_begonnen', 'mutation_abgeschlossen', 'rollback_abgeschlossen')) {
            $flag = $e.PSObject.Properties[$flagName]
            if ($null -eq $flag -or $flag.Value -isnot [bool]) {
                Abbruch "Journaleintrag $i hat kein typstrenges boolesches '$flagName'."
            }
        }
        if ($letzte.status -eq 'OK' -and
            (-not $e.mutation_begonnen -or -not $e.mutation_abgeschlossen -or $e.rollback_abgeschlossen)) {
            Abbruch "Journaleintrag $i widerspricht dem abgeschlossenen Status OK."
        }
        $ziel = Ziel-Pfad $a
        $zielAusJournal = try { Kanonischer-Pfad ([string]$e.ziel) } catch { Abbruch "Journalziel $i ist ungueltig." }
        if ($e.art -ne $a.art -or -not $zielAusJournal.Equals($ziel, [StringComparison]::OrdinalIgnoreCase)) {
            Abbruch "Journaleintrag $i passt nicht zum neu berechneten Manifestziel."
        }
        $sollId = if ($a.art -eq 'vst3') { [string]$a.ziel_id } else { '' }
        $journalId = if ($null -eq $e.ziel_id) { '' } else { [string]$e.ziel_id }
        if ($journalId -ne $sollId -or -not (Ist-Sha256 $e.sha256) -or
            ([string]$e.sha256).ToUpperInvariant() -ne ([string]$a.sha256).ToUpperInvariant()) {
            Abbruch "Journaleintrag $i hat eine fremde Ziel-ID oder ist nicht an den Manifest-Hash gebunden."
        }
        Fordere-Keine-Reparsepunkte $ziel 'Rueckwegziel'

        $sicherung = $null
        if ($null -ne $e.gesichert -and -not [string]::IsNullOrWhiteSpace([string]$e.gesichert)) {
            if (-not (Ist-Sha256 $e.vorher_sha256)) { Abbruch "Sicherung $i hat keinen Vorzustands-Hash." }
            $sicherung = Loese-GeschuetzteSicherung ([string]$e.gesichert) $aktiveTransaktionsId
            Fordere-GleichOderUnter $sicherung $aktiveSicherungen 'Sicherung'
            Fordere-Keine-Reparsepunkte $sicherung 'Sicherung'
            $typ = if ($a.art -eq 'vst3') { 'Container' } else { 'Leaf' }
            if (-not (Test-Path -LiteralPath $sicherung -PathType $typ)) { Abbruch "Sicherung fehlt: $sicherung" }
            $backupHash = Hash-Von $sicherung $a.art
            if ($backupHash -ne $e.vorher_sha256) {
                Abbruch "Sicherung $i stimmt nicht mit vorher_sha256 ueberein."
            }
            if ($a.art -eq 'vst3') {
                $innenHash = Datei-Hash (Innen-Pfad $a $sicherung)
                if (-not (Ist-Sha256 $innenHash) -or
                    $innenHash -ne ([string]$e.vorher_sha256_innen).ToUpperInvariant()) {
                    Abbruch "Sicherung $i stimmt nicht mit vorher_sha256_innen ueberein."
                }
            }
            elseif ($null -ne $e.vorher_sha256_innen) {
                Abbruch "Broker-Sicherung $i darf keinen inneren Bundle-Hash tragen."
            }
        }
        elseif ($null -ne $e.vorher_sha256) {
            Abbruch "Journaleintrag $i nennt einen Vorzustand ohne Sicherung."
        }

        $erzeugte = @()
        foreach ($d in @($e.erzeugte_ordner)) {
            $kanon = Kanonischer-Pfad ([string]$d)
            $erlaubt = Kanonischer-Pfad (Split-Path -Parent $ziel)
            if (-not $kanon.Equals($erlaubt, [StringComparison]::OrdinalIgnoreCase)) {
                Abbruch "Journal will ein nicht selbst beweisbares Verzeichnis entfernen: $kanon"
            }
            $dKey = $kanon.ToUpperInvariant()
            if ($erzeugteGesehen.ContainsKey($dKey)) { Abbruch "Erzeugtes Verzeichnis steht doppelt im Journal: $kanon" }
            $erzeugteGesehen[$dKey] = $true
            Fordere-Keine-Reparsepunkte $kanon 'erzeugtes Verzeichnis'
            $erzeugte += $kanon
        }

        $aktuell = Hash-Von $ziel $a.art
        $bereits = if ($null -ne $sicherung) { $aktuell -eq $e.vorher_sha256 }
                   else { $null -eq $aktuell }
        # Nur das GESCHUETZTE Journal darf einen sonst unbekannten Hash als
        # eigenen Crash-Zwischenstand belegen. Bei OK bleibt jede Abweichung
        # fremde Drift. VORBEREITET autorisiert nur das Fenster zwischen
        # `mutation_begonnen` und `mutation_abgeschlossen`; KOMPENSATION bzw.
        # ERROR_TEILSTAND nur einen noch nicht fertig rueckgefuehrten Eintrag.
        $eigenerTeilstand = (($letzte.status -eq 'VORBEREITET') -and
                             $e.mutation_begonnen -and -not $e.mutation_abgeschlossen) -or
                            (($letzte.status -in @('KOMPENSATION', 'ERROR_TEILSTAND', 'RUECKWEG_AKTIV')) -and
                             $e.mutation_begonnen -and -not $e.rollback_abgeschlossen)
        if (-not $bereits -and $aktuell -ne $e.sha256 -and -not $eigenerTeilstand) {
            Abbruch "Ziel $ziel wurde seit der Installation veraendert; Rueckweg wuerde fremden/neuen Stand ueberschreiben."
        }
        $validiert += [pscustomobject]@{
            Eintrag = $e; Artefakt = $a; Ziel = $ziel; Sicherung = $sicherung
            Erzeugte = $erzeugte; Bereits = $bereits; EigenerTeilstand = $eigenerTeilstand
        }
    }

    # NAK-41: Faellt ein Bundle auf einen Build mit KLEINEREM State-Schema
    # zurueck, verliert ein bereits als Schema 2 gespeichertes Projekt still
    # seine Messpunkt-Identitaet. Unbekannt zaehlt wie aelter.
    #
    # ZWEI HASHARTEN, EIN NACHSCHLAGEN (Vertrag §5.1, seit dem Umzug auf den
    # Ordner): die zwei historischen Eintraege wurden gegen die innere
    # BINAERDATEI eingefroren; neue Eintraege werden gegen den ORDNER
    # gebildet. Der Rueckweg schlaegt darum beide Zahlen nach - sonst faende
    # er den 16.08.-Stand nie wieder und verlangte bei jedem Rueckweg
    # -Erzwingen, also genau die Ermuedung, die einen Riegel wertlos macht.
    $bekanntOrdner = @{}
    $bekanntInnen  = @{}
    # Auch die State-Einstufung stammt aus dem geschuetzten Installationsmoment.
    # Sonst koennte eine spaet manipulierte Repo-Manifestzeile einen unbekannten
    # Vorzustand nachtraeglich als schema=2 deklarieren und NAK-41 umgehen.
    if ($letzte.PSObject.Properties.Name -notcontains 'bekannte_staende') {
        Abbruch 'Geschuetztes Transaktionsjournal enthaelt keine eingefrorenen bekannten State-Staende.'
    }
    foreach ($b in @($letzte.bekannte_staende)) {
        if (-not (Ist-Sha256 $b.sha256) -or $b.ziel_id -notin $sollIds -or
            ($b.state_schema -isnot [int] -and $b.state_schema -isnot [long]) -or
            [int64]$b.state_schema -lt 1 -or [int64]$b.state_schema -gt 2) {
            Abbruch 'Geschuetztes Transaktionsjournal enthaelt einen ungueltigen bekannten State-Stand.'
        }
        if ($b.hash_art -eq 'ordner')           { $bekanntOrdner[$b.sha256] = $b }
        elseif ($b.hash_art -eq 'datei-innen')  { $bekanntInnen[$b.sha256]  = $b }
        else { Abbruch "bekannter Stand $($b.sha256) hat kein gueltiges hash_art ('$($b.hash_art)')." }
    }

    $warnungen = @()
    foreach ($v in $validiert) {
        $e = $v.Eintrag
        if (-not $e.gesichert -or $v.Bereits) { continue } # kein Restore-Akt
        $ident = Ident-Von $e.ziel_id
        if ($null -eq $ident) { continue }            # Broker traegt keinen State
        # Der Vorstand IST diese Auslieferung - dieselben Bytes, also
        # dasselbe State-Schema. Ein zweiter Installationslauf ueber einen
        # bereits installierten Stand darf keine Warnung erzeugen: ein
        # Riegel, der bei einem No-Op ruft, erzieht dazu, -Erzwingen
        # reflexhaft mitzugeben - und das ist genau die Ermuedung, die ihn
        # wertlos macht. Der Vergleich ist exakt und raet nichts.
        if ($e.vorher_sha256 -and $e.vorher_sha256 -eq $e.sha256) { continue }
        $alt = Bekannt-Von $bekanntOrdner $e.vorher_sha256
        if ($null -eq $alt -and $e.PSObject.Properties.Name -contains 'vorher_sha256_innen') {
            $alt = Bekannt-Von $bekanntInnen $e.vorher_sha256_innen
        }
        $altSchema = if ($alt) { $alt.state_schema } else { $null }
        if ($null -eq $altSchema -or $altSchema -lt $ident.state_schema) {
            $gezeigt = if ($null -eq $altSchema) { 'unbekannt' } else { "$altSchema" }
            $warnungen += "$($ident.produktname): zurueck auf State-Schema $gezeigt (jetzt: $($ident.state_schema))"
        }
    }

    if ($warnungen.Count -gt 0) {
        Write-Host ''
        Write-Host 'NAK-41 — dieser Rueckweg kostet Projektstaende:' -ForegroundColor Yellow
        foreach ($w in $warnungen) { Write-Host "  - $w" -ForegroundColor Yellow }
        Write-Host '  Ein Projekt, das der neue Build als NakamaState schema=2 gespeichert hat,' -ForegroundColor Yellow
        Write-Host '  wird vom aelteren Build als fremder Baum IGNORIERT - ohne Fehlermeldung.' -ForegroundColor Yellow
        if (-not $Erzwingen) {
            Abbruch 'Rueckweg verweigert (NAK-41). Mit -Erzwingen ausdruecklich wiederholen.'
        }
        Write-Host '  -Erzwingen gesetzt: der Rueckweg laeuft trotzdem.' -ForegroundColor Yellow
    }

    if ($Pruefen) {
        foreach ($v in $validiert) { Write-Host "  wuerde zurueckstellen: $($v.Ziel)" }
        exit 0
    }

    # Der inverse Pfad ist selbst eine Transaktion. Stirbt er mitten im
    # Restore, autorisiert dieser geschuetzte Status beim naechsten Lauf genau
    # die noch nicht als rueckgefuehrt markierten Ziele als eigene Teilstaende.
    $letzte.status = 'RUECKWEG_AKTIV'
    Schreibe-Ergebnis $letzte
    $getan = @()
    $rueckwaerts = @($validiert)
    [array]::Reverse($rueckwaerts)
    foreach ($v in $rueckwaerts) {
        $e = $v.Eintrag
        $art = $v.Artefakt.art
        if (-not $v.Bereits) {
            if ($null -ne $v.Sicherung) {
                Stelle-Gepruefte-SicherungWiederHer $v.Sicherung $v.Ziel $art $e.vorher_sha256
                $getan += "wiederhergestellt: $($v.Ziel)"
            }
            elseif (Entferne-Stand $v.Ziel $art) {
                if ($null -ne (Hash-Von $v.Ziel $art)) { throw "Entfernen blieb unvollstaendig: $($v.Ziel)" }
                $getan += "entfernt: $($v.Ziel)"
            }
        }

        foreach ($d in @($v.Erzeugte)) {
            if (-not (Test-Path -LiteralPath $d -PathType Container)) { continue }
            if (@(Get-ChildItem -LiteralPath $d -Force).Count -gt 0) {
                Write-Host "  bleibt (nicht leer): $d" -ForegroundColor DarkGray
                continue
            }
            Remove-Item -LiteralPath $d -Force
            $getan += "Ordner entfernt: $d"
        }
        $e.rollback_abgeschlossen = $true
        Schreibe-Ergebnis $letzte
    }

    foreach ($g in $getan) { Write-Host "  $g" }
    Schreibe-Ergebnis ([pscustomobject]@{
        schema    = $journalSchema
        status    = 'RUECKWEG'
        transaktions_id = $aktiveTransaktionsId
        erzwungen = [bool]$Erzwingen
        warnungen = $warnungen
        getan     = $getan
        zeit      = [DateTime]::UtcNow.ToString('o')
    })
    # Die oeffentliche Ergebnisdatei bleibt als Diagnose stehen. Der
    # geschuetzte aktive Anker wird erst NACH vollstaendig erfolgreichem
    # Gegenpfad entfernt und kann danach keinen zweiten Restore autorisieren.
    Remove-Item -LiteralPath $transaktionsWeg -Force
    Entferne-TransaktionsSicherungen $aktiveTransaktionsId
    Bereinige-VerwaisteSicherungen $null
    if ($null -ne $script:geschuetzterInstallerLock) {
        $script:geschuetzterInstallerLock.Dispose()
        $script:geschuetzterInstallerLock = $null
    }
    Entferne-LeereTransaktionsWurzeln
    Write-Host 'RUECKWEG OK' -ForegroundColor Green
    exit 0
}

# ═══════════════════════════════════════════ INSTALLIEREN / PRUEFEN ════════

# Riegel 1: Vollstaendigkeit. Vor allem anderen.
$ohneHash = @($manifest.artefakte | Where-Object { -not (Ist-Sha256 $_.sha256) })
if ($ohneHash.Count -gt 0) {
    $namen = ($ohneHash | ForEach-Object { Artefakt-Name $_ }) -join ', '
    Abbruch "Nicht ausliefer-bar: $($ohneHash.Count) Artefakt(e) ohne festgeschriebenen Hash ($namen). Erst `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen` nach einem gruenen Kanon-Lauf."
}

# Riegel 2: Echtheit der Quellen - komplett, bevor irgendetwas kopiert wird.
$plan = @()
foreach ($a in $manifest.artefakte) {
    $quelle = Loese-Pfad $a.quelle
    $name   = Artefakt-Name $a
    $was    = if ($a.art -eq 'vst3') { 'Bundle-Ordner' } else { 'Quelldatei' }
    $typ    = if ($a.art -eq 'vst3') { 'Container' } else { 'Leaf' }
    if (-not (Test-Path -LiteralPath $quelle -PathType $typ)) { Abbruch "$name : $was fehlt ($($a.quelle))." }
    Fordere-Keine-Reparsepunkte $quelle 'Installationsquelle'
    foreach ($zielBasis in @($vst3Basis, $brokerBasis)) {
        if ((Ist-GleichOderUnter $quelle $zielBasis) -or
            (Ist-GleichOderUnter $zielBasis $quelle)) {
            Abbruch "$name : Quelle und Installationsziel duerfen nicht ineinander liegen."
        }
    }
    $ist = Hash-Von $quelle $a.art
    if ($ist -ne $a.sha256) {
        Abbruch "$name : Quell-Hash weicht ab. Manifest $($a.sha256), gebaut $ist. Entweder ist der Bau neuer als das Manifest oder der Stand ist nicht der, der eingefroren wurde."
    }
    $plan += [pscustomobject]@{
        Artefakt = $a
        Name     = $name
        Quelle   = $quelle
        Ziel     = Ziel-Pfad $a
    }
    Write-Host "  ok      $name : Quelle echt  [$($a.sha256.Substring(0,16))]"
}

# Signatur: nur behaupten, was geprueft wird.
if ($null -ne $manifest.signatur.authenticode_thumbprint) {
    $thumbprint = ([string]$manifest.signatur.authenticode_thumbprint).ToUpperInvariant()
    if ($thumbprint -notmatch '^(?:[0-9A-F]{40}|[0-9A-F]{64})$') {
        Abbruch 'authenticode_thumbprint muss ein SHA-1- oder SHA-256-Zertifikatfingerabdruck sein.'
    }
    foreach ($p in $plan) {
        $signaturPfad = if ($p.Artefakt.art -eq 'vst3') {
            Innen-Pfad $p.Artefakt $p.Quelle
        }
        else { $p.Quelle }
        if (-not (Test-Path -LiteralPath $signaturPfad -PathType Leaf)) {
            Abbruch "$($p.Name): signierbare Binaerdatei fehlt im Artefakt."
        }
        $sig = Get-AuthenticodeSignature -LiteralPath $signaturPfad
        if ($sig.Status -ne 'Valid' -or
            ([string]$sig.SignerCertificate.Thumbprint).ToUpperInvariant() -ne $thumbprint) {
            Abbruch "$($p.Name): Authenticode passt nicht zum Manifest-Thumbprint."
        }
    }
    Write-Host '  ok      Authenticode geprueft'
}
else {
    Write-Host '  hinweis Authenticode wird NICHT geprueft - es gibt kein Zertifikat (siehe signatur.warum_null).' -ForegroundColor DarkGray
}

Melde-StillgelegteAltlasten

if ($Pruefen) {
    Write-Host ''
    Write-Host 'Installierter Stand gegen das Manifest:'
    foreach ($p in $plan) {
        # Der ORDNER wird gehasht, nicht die Binaerdatei darin: sonst hiesse
        # `aktuell` nur "die .vst3-Datei stimmt" und ein veraltetes
        # moduleinfo.json daneben bliebe unsichtbar (Befund T2-5 (c)).
        $ist = Hash-Von $p.Ziel $p.Artefakt.art
        $urteil = if ($null -eq $ist) { 'nicht installiert' }
                  elseif ($ist -eq $p.Artefakt.sha256) { 'aktuell' }
                  else { 'ABWEICHEND' }
        Write-Host ("  {0,-16} {1}" -f $urteil, $p.Ziel)
    }
    exit 0
}

# Riegel 3: Ruhe.
if (Fl-Laeuft) { Abbruch 'FL Studio laeuft noch. Installation abgebrochen.' }

# Gegenpfad: ALLE Vorstaende sichern und pruefen, dann erst den ersten Stand
# anfassen. Das Journal steht vor jeder Mutation auf VORBEREITET. Stirbt der
# Prozess hart, kann -Rueckweg diesen Stand lesen; faengt PowerShell den Fehler,
# wird die ganze Serie sofort rueckwaerts kompensiert.
Sperre-GeschuetztenInstaller
if ((Test-Path -LiteralPath $transaktionsWeg) -and
    -not (Test-Path -LiteralPath $transaktionsWeg -PathType Leaf)) {
    Abbruch "Geschuetzter Transaktionsanker ist keine Datei: $transaktionsWeg"
}

# Ein unfertiger geschuetzter Plan ist die einzige Recovery-Autoritaet und
# darf nie von einem neuen Lauf ueberschrieben werden. Ein abgeschlossener
# OK-Stand darf erst superseded werden, nachdem die neue Transaktion ihren
# eigenen Vorzustand vollstaendig gesichert und atomar veroeffentlicht hat.
$vorherigeTransaktionsId = $null
if (Test-Path -LiteralPath $transaktionsWeg -PathType Leaf) {
    Fordere-Keine-Reparsepunkte $transaktionsWeg 'vorhandener geschuetzter Transaktionsanker'
    try { $vorherigerAnker = Get-Content -LiteralPath $transaktionsWeg -Raw | ConvertFrom-Json }
    catch { Abbruch "Vorhandener geschuetzter Transaktionsanker ist unlesbar: $($_.Exception.Message)" }
    if ($vorherigerAnker.schema -ne $journalSchema) {
        Abbruch "Vorhandener geschuetzter Transaktionsanker hat ein fremdes Schema; er wird nicht ueberschrieben."
    }
    if ($vorherigerAnker.status -in @('VORBEREITET', 'KOMPENSATION', 'ERROR_TEILSTAND', 'RUECKWEG_AKTIV')) {
        Abbruch "Unfertige Transaktion '$($vorherigerAnker.status)' gefunden. Erst -Rueckweg ausfuehren; der Recovery-Plan bleibt unangetastet."
    }
    if ($vorherigerAnker.status -notin @('OK', 'ERROR_RUECKGEROLLT', 'RUECKWEG')) {
        Abbruch "Vorhandener geschuetzter Transaktionsanker hat den unbekannten Status '$($vorherigerAnker.status)' und wird nicht ueberschrieben."
    }
    if (-not (Ist-TransaktionsId $vorherigerAnker.transaktions_id)) {
        Abbruch 'Abgeschlossener geschuetzter Transaktionsanker hat keine gueltige Transaktions-ID.'
    }
    $vorherigeTransaktionsId = [string]$vorherigerAnker.transaktions_id

    if ($vorherigerAnker.status -in @('OK', 'ERROR_RUECKGEROLLT')) {
        foreach ($e in @($vorherigerAnker.eintraege)) {
            foreach ($flagName in @('mutation_begonnen', 'mutation_abgeschlossen', 'rollback_abgeschlossen')) {
                $flag = $e.PSObject.Properties[$flagName]
                if ($null -eq $flag -or $flag.Value -isnot [bool]) {
                    Abbruch "Abgeschlossener Anker hat kein typstrenges boolesches '$flagName'."
                }
            }
            if ($vorherigerAnker.status -eq 'OK' -and
                (-not $e.mutation_begonnen -or -not $e.mutation_abgeschlossen -or $e.rollback_abgeschlossen)) {
                Abbruch 'Status OK widerspricht seinen Mutationsmarken und wird nicht superseded.'
            }
            if ($vorherigerAnker.status -eq 'ERROR_RUECKGEROLLT' -and
                $e.mutation_begonnen -and -not $e.rollback_abgeschlossen) {
                Abbruch 'Status ERROR_RUECKGEROLLT enthaelt einen offenen Gegenakt und wird nicht superseded.'
            }
        }
    }
    else {
        # RUECKWEG ist der bereits vollstaendig veroeffentlichte Endstand. Ein
        # Crash vor dem anschliessenden Cleanup darf den naechsten Lauf nicht
        # blockieren und autorisiert keinen zweiten Restore.
        Remove-Item -LiteralPath $transaktionsWeg -Force
        Entferne-TransaktionsSicherungen $vorherigeTransaktionsId
        $vorherigeTransaktionsId = $null
    }
}

Fordere-Keine-Reparsepunkte $transaktionsOrdner 'geschuetzte Transaktionswurzel'
if (-not (Test-Path -LiteralPath $sicherungsOrdner)) {
    New-Item -ItemType Directory -Path $sicherungsOrdner -Force | Out-Null
}
Fordere-Keine-Reparsepunkte $sicherungsOrdner 'geschuetzte Sicherungswurzel'
Bereinige-VerwaisteSicherungen $vorherigeTransaktionsId

# Ein vorhandenes Bundle ist nur dann ersetzbar, wenn neben seinem gesamten
# Ordnerhash auch die erwartete innere Payload beweisbar ist. Der Rueckweg
# bindet historische bekannte Staende an genau diesen Hash; `null` wuerde erst
# nach der Installation auffallen und einen nicht restaurierbaren Vorzustand
# erzeugen. Diese Vollvorpruefung liegt vor Transaktionsordner und Sicherungen.
foreach ($p in $plan) {
    if ($p.Artefakt.art -ne 'vst3') { continue }
    $vorher = Hash-Von $p.Ziel $p.Artefakt.art
    if ($null -eq $vorher) { continue }
    $vorherInnen = Datei-Hash (Innen-Pfad $p.Artefakt $p.Ziel)
    if (-not (Ist-Sha256 $vorherInnen)) {
        Abbruch "$($p.Name): vorhandenes VST3-Bundle hat keine beweisbare erwartete innere Payload; Installation bleibt unveraendert."
    }
}

$transaktionsId = [Guid]::NewGuid().ToString('N')
$transaktionsSicherungsOrdner = Sicherungsordner-Fuer $transaktionsId
New-Item -ItemType Directory -Path $transaktionsSicherungsOrdner | Out-Null
Fordere-Keine-Reparsepunkte $transaktionsSicherungsOrdner 'Sicherungen der neuen Transaktion'

$eintraege = @()
for ($planIndex = 0; $planIndex -lt $plan.Count; ++$planIndex) {
    $p = $plan[$planIndex]
    $art    = $p.Artefakt.art
    $vorher = Hash-Von $p.Ziel $art
    # Die innere Binaerdatei zusaetzlich: NUR damit der Rueckweg die zwei
    # historischen `bekannte_staende`-Eintraege wiederfindet, die gegen sie
    # eingefroren wurden (Vertrag §5.1).
    $vorherInnen = $null
    $gesichert   = $null
    $sicherung   = $null
    if ($null -ne $vorher) {
        if ($art -eq 'vst3') {
            $vorherInnen = Datei-Hash (Innen-Pfad $p.Artefakt $p.Ziel)
            # Gegen eine externe Aenderung zwischen Vollvorpruefung und
            # Sicherungszug: auch hier nie ein null-gebundenes Journal bauen.
            if (-not (Ist-Sha256 $vorherInnen)) {
                Abbruch "$($p.Name): VST3-Payload ist vor der Sicherung verschwunden; Installation bleibt ohne Zielmutation."
            }
        }
        $endung  = if ($art -eq 'vst3') { '.bundle' } else { '.bak' }
        $sicherungsName = "backup-$planIndex$endung"
        $sicherung = Kanonischer-Pfad (Join-Path $transaktionsSicherungsOrdner $sicherungsName)
        if (-not (Kanonischer-Pfad (Split-Path -Parent $sicherung)).Equals(
                $transaktionsSicherungsOrdner, [StringComparison]::OrdinalIgnoreCase)) {
            Abbruch "Sicherung verlaesst ihren geschuetzten Transaktionsordner: $sicherung"
        }
        Kopiere-Stand $p.Ziel $sicherung $art
        if ((Hash-Von $sicherung $art) -ne $vorher) {
            Abbruch "$($p.Name): Sicherung stimmt nicht mit dem Vorzustand ueberein."
        }
        # Relativ zur GESCHUETZTEN Transaktionswurzel, nie zum Benutzer-Repo.
        $gesichert = "backups/$transaktionsId/$sicherungsName"
    }

    $eintraege += [pscustomobject]@{
        ziel_id                = $p.Artefakt.ziel_id
        art                    = $art
        name                   = $p.Name
        ziel                   = $p.Ziel
        sha256                 = $p.Artefakt.sha256
        vorher_sha256          = $vorher
        vorher_sha256_innen    = $vorherInnen
        gesichert              = $gesichert
        # Erst NACH erfolgreichem New-Item als unser Ordner protokollieren.
        # Ein Crash im schmalen Fenster laesst lieber einen leeren Ordner als
        # spaeter einen inzwischen fremden Ordner zu entfernen.
        erzeugte_ordner        = @()
        mutation_begonnen      = $false
        mutation_abgeschlossen = $false
        rollback_abgeschlossen = $false
    }
}

$journal = [pscustomobject]@{
    schema    = $journalSchema
    status    = 'VORBEREITET'
    transaktions_id = $transaktionsId
    manifest  = 'eq-copilot/install/nakama-installer-v1.json'
    zeit      = [DateTime]::UtcNow.ToString('o')
    bekannte_staende = @($manifest.rueckweg.bekannte_staende)
    eintraege = $eintraege
}
Schreibe-Ergebnis $journal
if ($null -ne $vorherigeTransaktionsId -and $vorherigeTransaktionsId -ne $transaktionsId) {
    Entferne-TransaktionsSicherungen $vorherigeTransaktionsId
}
Bereinige-VerwaisteSicherungen $transaktionsId

try {
    for ($i = 0; $i -lt $plan.Count; ++$i) {
        $p = $plan[$i]
        $e = $eintraege[$i]
        $art = $p.Artefakt.art

        if ((Hash-Von $p.Ziel $art) -ne $e.vorher_sha256) {
            throw "$($p.Name): Ziel wurde zwischen Sicherung und Tausch veraendert."
        }

        # Ab hier kann ein harter Abbruch einen halben Stand hinterlassen. Das
        # vorab geschriebene Flag macht den Gegenpfad dafuer eindeutig.
        $e.mutation_begonnen = $true
        Schreibe-Ergebnis $journal

        $tatsaechlichErzeugt = @(Erzeuge-Ordnerkette (Split-Path -Parent $p.Ziel))
        $e.erzeugte_ordner = $tatsaechlichErzeugt
        Schreibe-Ergebnis $journal

        # Ersetzen, nicht mischen: Kopiere-Stand entfernt einen vorhandenen
        # Stand zuerst. Auf einen bestehenden Ordner zu kopieren fuehrte sonst
        # zusammen und liess Altdateien im Bundle ueberleben.
        Kopiere-Stand $p.Quelle $p.Ziel $art

        $nachher = Hash-Von $p.Ziel $art
        if ($nachher -ne $p.Artefakt.sha256) {
            throw "$($p.Name): installierter Stand stimmt nicht ($nachher)."
        }
        $e.mutation_abgeschlossen = $true
        Schreibe-Ergebnis $journal
        Write-Host "  ok      $($p.Name) installiert  [$($p.Ziel)]"
    }

    $journal.status = 'OK'
    $journal.zeit = [DateTime]::UtcNow.ToString('o')
    Schreibe-Ergebnis $journal
    Write-Host 'INSTALLATION OK' -ForegroundColor Green
    exit 0
}
catch {
    $installFehler = $_.Exception.Message
    $kompensationsFehler = @()
    # Ab hier kann ein harter Abbruch nicht mehr wie ein Install-Crash gelesen
    # werden: auch ein zuvor vollstaendig installiertes Ziel kann gerade halb
    # in den Vorzustand zurueckkopiert sein. Der geschuetzte Status wird vor
    # dem ersten Gegenakt und danach pro Eintrag fortgeschrieben.
    $journal.status = 'KOMPENSATION'
    try { Schreibe-Ergebnis $journal }
    catch { $kompensationsFehler += "Kompensationsjournal konnte nicht begonnen werden: $($_.Exception.Message)" }
    for ($i = $eintraege.Count - 1; $i -ge 0; --$i) {
        $e = $eintraege[$i]
        if (-not $e.mutation_begonnen) { continue }
        $art = $e.art
        try {
            # Diese Eintraege stammen aus dem soeben in diesem Prozess
            # aufgebauten Plan. Erneut gegen Reparse-Rennen pruefen, hier aber
            # WERFEN statt `Abbruch`/`exit`, damit die restlichen Ziele weiter
            # kompensiert und der Teilstand journalisiert werden.
            $lauf = Kanonischer-Pfad ([string]$e.ziel)
            while (-not [string]::IsNullOrEmpty($lauf)) {
                if (Test-Path -LiteralPath $lauf) {
                    $item = Get-Item -LiteralPath $lauf -Force
                    if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                        throw "Kompensationsziel enthaelt einen Reparsepunkt: $lauf"
                    }
                }
                $eltern = Split-Path -Parent $lauf
                if ([string]::IsNullOrEmpty($eltern) -or $eltern -eq $lauf) { break }
                $lauf = $eltern
            }
            if ($null -ne $e.gesichert) {
                $relativ = [string]$e.gesichert
                if ([IO.Path]::IsPathRooted($relativ)) { throw "Kompensationssicherung ist nicht relativ: $relativ" }
                $sicherung = Kanonischer-Pfad (Join-Path $transaktionsOrdner ($relativ -replace '/', '\'))
                if (-not (Kanonischer-Pfad (Split-Path -Parent $sicherung)).Equals(
                        $transaktionsSicherungsOrdner, [StringComparison]::OrdinalIgnoreCase)) {
                    throw "Kompensationssicherung verlaesst ihren geschuetzten Transaktionsordner: $sicherung"
                }
                if ((Hash-Von $sicherung $art) -ne $e.vorher_sha256) {
                    throw "Sicherung wurde nach der Vorpruefung veraendert: $sicherung"
                }
                Stelle-Gepruefte-SicherungWiederHer $sicherung ([string]$e.ziel) $art $e.vorher_sha256
            }
            else {
                Entferne-Stand ([string]$e.ziel) $art | Out-Null
                if ($null -ne (Hash-Von ([string]$e.ziel) $art)) {
                    throw "Kompensation konnte den neu installierten Stand nicht entfernen: $($e.ziel)"
                }
            }
            foreach ($d in @($e.erzeugte_ordner)) {
                if ((Test-Path -LiteralPath $d -PathType Container) -and
                    @(Get-ChildItem -LiteralPath $d -Force).Count -eq 0) {
                    Remove-Item -LiteralPath $d -Force
                }
            }
            $e.rollback_abgeschlossen = $true
            Schreibe-Ergebnis $journal
        }
        catch {
            $kompensationsFehler += $_.Exception.Message
        }
    }

    $journal.status = if ($kompensationsFehler.Count -eq 0) { 'ERROR_RUECKGEROLLT' } else { 'ERROR_TEILSTAND' }
    $journal.zeit = [DateTime]::UtcNow.ToString('o')
    $journal | Add-Member -NotePropertyName fehler -NotePropertyValue $installFehler -Force
    $journal | Add-Member -NotePropertyName kompensationsfehler -NotePropertyValue $kompensationsFehler -Force
    try { Schreibe-Ergebnis $journal } catch { $kompensationsFehler += $_.Exception.Message }

    if ($journal.status -eq 'ERROR_RUECKGEROLLT' -and $kompensationsFehler.Count -eq 0) {
        # Der Vorzustand ist vollstaendig wiederhergestellt. Der geschuetzte
        # Endanker bleibt als expliziter Supersede-Stand; seine nun nutzlosen
        # Bytes werden sofort entfernt und wachsen nicht je Fehlversuch weiter.
        try {
            Entferne-TransaktionsSicherungen $transaktionsId
            Bereinige-VerwaisteSicherungen $null
        }
        catch {
            # Kein Teilstand: die Ziele sind bereits vollstaendig restauriert
            # und der geschuetzte Endanker bleibt. Der naechste sichere
            # Supersede-Lauf bereinigt die verwaiste Generation erneut.
            Write-Host "  HINWEIS: Backup-Cleanup wird beim naechsten Lauf wiederholt: $($_.Exception.Message)" -ForegroundColor Yellow
        }
    }

    Write-Host "ABBRUCH: Installation fehlgeschlagen und wurde kompensiert: $installFehler" -ForegroundColor Red
    foreach ($f in $kompensationsFehler) { Write-Host "  KOMPENSATIONSFEHLER: $f" -ForegroundColor Red }
    exit 1
}
