<#
.SYNOPSIS
    S9/SONDE-007b Abschnitt 3 - Installation der Sondenfamilie aus dem
    Installer-Manifest. Beide Haelften: installieren UND Rueckweg.

.DESCRIPTION
    Dieses Skript entscheidet NICHTS. Es liest
    eq-copilot/install/nakama-installer-v1.json (was ausgeliefert wird) und
    eq-copilot/identity/plugin-identities-v1.json (wie die Bundles heissen)
    und tut genau das, was dort steht. Kein Bundlename, kein Pfad und kein
    Hash steht in diesem Skript - das war der Fehler der Vorgaengerfassung
    Install-EQ-Copilot.ps1, die drei Hashes und zwei Pfade als Literale trug
    und mit jedem Bau von Hand nachgezogen werden musste.

    DREI RIEGEL, VOR JEDEM SCHREIBZUGRIFF:
      1. Vollstaendigkeit - ein Artefakt mit `sha256: null` bricht ab. Ein
         halb eingefrorenes Paket ist kein Paket.
      2. Echtheit - jede Quelldatei wird gegen ihren Manifest-Hash geprueft,
         BEVOR irgendetwas kopiert wird. Und danach noch einmal am Ziel.
      3. Ruhe - laeuft FL Studio, wird nichts angefasst. Ein Host, der ein
         Bundle geladen haelt, bekommt sonst eine Datei unter den Fingern
         weggetauscht.

    GEGENPFAD (Hausinvariante: installieren <-> Rollback im selben
    Aenderungssatz): vor jedem Tausch wandert die VORGEFUNDENE Datei nach
    install/rueckweg/ und ihr Hash in install/install-ergebnis.json.
    `-Rueckweg` stellt genau diesen Stand wieder her - und verweigert, wenn
    er NAK-41 ausloesen wuerde (Rueckfall auf einen Build mit kleinerem
    State-Schema: das Projekt verliert dann still seine Messpunkt-Identitaet).

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

function Loese-Pfad([string]$relativ) { Join-Path $wurzel ($relativ -replace '/', '\') }

function Schreibe-Ergebnis($stand) {
    $stand | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ergebnisWeg -Encoding UTF8
}

function Abbruch([string]$grund) {
    Write-Host "ABBRUCH: $grund" -ForegroundColor Red
    # Ein -Pruefen-Lauf schreibt NICHTS, auch kein Fehlerprotokoll: sonst
    # ueberschriebe eine Diagnose den Rueckweg-Stand der letzten echten
    # Installation, und der Gegenpfad haette keine Quelle mehr.
    if (-not $Pruefen) {
        Schreibe-Ergebnis ([pscustomobject]@{
            status = 'ERROR'
            grund  = $grund
            zeit   = [DateTime]::UtcNow.ToString('o')
        })
    }
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

# Ziel-ID -> Identitaetseintrag. Der Bundlename kommt AUSSCHLIESSLICH von hier.
$zieleNachId = @{}
foreach ($z in $identitaet.ziele) { $zieleNachId[$z.id] = $z }

function Ziel-Pfad($artefakt) {
    if ($artefakt.art -eq 'vst3') {
        $ident = $zieleNachId[$artefakt.ziel_id]
        if ($null -eq $ident) { Abbruch "Ziel '$($artefakt.ziel_id)' steht nicht in der Identitaetsdatei." }
        $bundle = $ident.bundle
        return Join-Path ($manifest.ziele.vst3_verzeichnis -replace '/', '\') `
                         (Join-Path $bundle (Join-Path 'Contents\x86_64-win' $bundle))
    }
    return Join-Path ($manifest.ziele.broker_verzeichnis -replace '/', '\') $artefakt.name
}

function Artefakt-Name($artefakt) {
    if ($artefakt.art -eq 'vst3') { return $zieleNachId[$artefakt.ziel_id].produktname }
    return $artefakt.name
}

function Hash-Von([string]$pfad) {
    if (-not (Test-Path -LiteralPath $pfad)) { return $null }
    return (Get-FileHash -LiteralPath $pfad -Algorithm SHA256).Hash
}

function Fl-Laeuft {
    $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
}

# ═══════════════════════════════════════════════════ RUECKWEG ══════════════
if ($Rueckweg) {
    if (-not (Test-Path -LiteralPath $ergebnisWeg)) {
        Abbruch 'Kein install-ergebnis.json - es gibt keinen gesicherten Stand, auf den zurueckzugehen waere.'
    }
    $letzte = Get-Content -LiteralPath $ergebnisWeg -Raw | ConvertFrom-Json
    if ($letzte.status -ne 'OK') { Abbruch "Der letzte Lauf endete mit '$($letzte.status)' - nichts zu sichern." }
    if (Fl-Laeuft) { Abbruch 'FL Studio laeuft. Der Rueckweg fasst nichts an.' }

    # NAK-41: Faellt ein Bundle auf einen Build mit KLEINEREM State-Schema
    # zurueck, verliert ein bereits als Schema 2 gespeichertes Projekt still
    # seine Messpunkt-Identitaet. Unbekannt zaehlt wie aelter.
    $bekannt = @{}
    foreach ($b in $manifest.rueckweg.bekannte_staende) { $bekannt[$b.sha256] = $b }

    $warnungen = @()
    foreach ($e in $letzte.eintraege) {
        if (-not $e.gesichert) { continue }           # war vorher nicht da
        $ident = $zieleNachId[$e.ziel_id]
        if ($null -eq $ident) { continue }            # Broker traegt keinen State
        $alt = $bekannt[$e.vorher_sha256]
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

    $getan = @()
    foreach ($e in $letzte.eintraege) {
        if ($Pruefen) { Write-Host "  wuerde zurueckstellen: $($e.ziel)"; continue }
        if ($e.gesichert) {
            Copy-Item -LiteralPath (Loese-Pfad $e.gesichert) -Destination $e.ziel -Force
            $getan += "wiederhergestellt: $($e.ziel)"
        }
        elseif (Test-Path -LiteralPath $e.ziel) {
            # Vorher gab es die Datei nicht — der Rueckweg ist das Entfernen.
            Remove-Item -LiteralPath $e.ziel -Force
            $getan += "entfernt: $($e.ziel)"
        }
    }
    if ($Pruefen) { exit 0 }

    foreach ($g in $getan) { Write-Host "  $g" }
    Schreibe-Ergebnis ([pscustomobject]@{
        status    = 'RUECKWEG'
        erzwungen = [bool]$Erzwingen
        warnungen = $warnungen
        getan     = $getan
        zeit      = [DateTime]::UtcNow.ToString('o')
    })
    Write-Host 'RUECKWEG OK' -ForegroundColor Green
    exit 0
}

# ═══════════════════════════════════════════ INSTALLIEREN / PRUEFEN ════════

# Riegel 1: Vollstaendigkeit. Vor allem anderen.
$ohneHash = @($manifest.artefakte | Where-Object { $null -eq $_.sha256 })
if ($ohneHash.Count -gt 0) {
    $namen = ($ohneHash | ForEach-Object { Artefakt-Name $_ }) -join ', '
    Abbruch "Nicht ausliefer-bar: $($ohneHash.Count) Artefakt(e) ohne festgeschriebenen Hash ($namen). Erst `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen` nach einem gruenen Kanon-Lauf."
}

# Riegel 2: Echtheit der Quellen - komplett, bevor irgendetwas kopiert wird.
$plan = @()
foreach ($a in $manifest.artefakte) {
    $quelle = Loese-Pfad $a.quelle
    $name   = Artefakt-Name $a
    if (-not (Test-Path -LiteralPath $quelle)) { Abbruch "$name : Quelldatei fehlt ($($a.quelle))." }
    $ist = Hash-Von $quelle
    if ($ist -ne $a.sha256) {
        Abbruch "$name : Quell-Hash weicht ab. Manifest $($a.sha256), Datei $ist. Entweder ist der Bau neuer als das Manifest oder die Datei ist nicht die, die eingefroren wurde."
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
    foreach ($p in $plan) {
        $sig = Get-AuthenticodeSignature -LiteralPath $p.Quelle
        if ($sig.Status -ne 'Valid' -or $sig.SignerCertificate.Thumbprint -ne $manifest.signatur.authenticode_thumbprint) {
            Abbruch "$($p.Name): Authenticode passt nicht zum Manifest-Thumbprint."
        }
    }
    Write-Host '  ok      Authenticode geprueft'
}
else {
    Write-Host '  hinweis Authenticode wird NICHT geprueft - es gibt kein Zertifikat (siehe signatur.warum_null).' -ForegroundColor DarkGray
}

if ($Pruefen) {
    Write-Host ''
    Write-Host 'Installierter Stand gegen das Manifest:'
    foreach ($p in $plan) {
        $ist = Hash-Von $p.Ziel
        $urteil = if ($null -eq $ist) { 'nicht installiert' }
                  elseif ($ist -eq $p.Artefakt.sha256) { 'aktuell' }
                  else { 'ABWEICHEND' }
        Write-Host ("  {0,-16} {1}" -f $urteil, $p.Ziel)
    }
    exit 0
}

# Riegel 3: Ruhe.
if (Fl-Laeuft) { Abbruch 'FL Studio laeuft noch. Installation abgebrochen.' }

# Gegenpfad: erst sichern, dann tauschen.
$sicherungsOrdner = Join-Path $PSScriptRoot 'rueckweg'
if (-not (Test-Path -LiteralPath $sicherungsOrdner)) {
    New-Item -ItemType Directory -Path $sicherungsOrdner -Force | Out-Null
}
$stempel = [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')

$eintraege = @()
foreach ($p in $plan) {
    $vorher = Hash-Von $p.Ziel
    $gesichert = $null
    if ($null -ne $vorher) {
        $kennung = if ($p.Artefakt.art -eq 'vst3') { $p.Artefakt.ziel_id } else { 'broker' }
        $sicherung = Join-Path $sicherungsOrdner "$stempel-$kennung.bak"
        Copy-Item -LiteralPath $p.Ziel -Destination $sicherung -Force
        # Relativ speichern: die Ergebnisdatei soll einen Workspace-Umzug ueberleben.
        $gesichert = 'eq-copilot/install/rueckweg/' + (Split-Path -Leaf $sicherung)
    }

    $zielOrdner = Split-Path -Parent $p.Ziel
    if (-not (Test-Path -LiteralPath $zielOrdner)) {
        New-Item -ItemType Directory -Path $zielOrdner -Force | Out-Null
    }
    Copy-Item -LiteralPath $p.Quelle -Destination $p.Ziel -Force

    $nachher = Hash-Von $p.Ziel
    if ($nachher -ne $p.Artefakt.sha256) {
        Abbruch "$($p.Name): installierte Datei stimmt nicht ($nachher). Der Rueckweg liegt in $gesichert."
    }
    Write-Host "  ok      $($p.Name) installiert  [$($p.Ziel)]"

    $eintraege += [pscustomobject]@{
        ziel_id       = $p.Artefakt.ziel_id
        name          = $p.Name
        ziel          = $p.Ziel
        sha256        = $nachher
        vorher_sha256 = $vorher
        gesichert     = $gesichert
    }
}

Schreibe-Ergebnis ([pscustomobject]@{
    status    = 'OK'
    manifest  = 'eq-copilot/install/nakama-installer-v1.json'
    zeit      = [DateTime]::UtcNow.ToString('o')
    eintraege = $eintraege
})
Write-Host 'INSTALLATION OK' -ForegroundColor Green
exit 0
