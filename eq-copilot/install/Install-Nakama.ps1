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
    Aenderungssatz): vor jedem Tausch wandert der VORGEFUNDENE Stand nach
    install/rueckweg/ und sein Hash in install/install-ergebnis.json.
    `-Rueckweg` stellt genau diesen Stand wieder her - und verweigert, wenn
    er NAK-41 ausloesen wuerde (Rueckfall auf einen Build mit kleinerem
    State-Schema: das Projekt verliert dann still seine Messpunkt-Identitaet).

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

# Ordner-Hash v1 (Vertrag §2.1). Dot-Source, weil dieselbe Funktion auch die
# Kreuzprobe von A17 ruft: waere sie hier eingebettet, koennte die Kreuzprobe
# nur eine Kopie messen, und zwei Kopien laufen auseinander.
$ordnerHashWeg = Join-Path $PSScriptRoot 'NakamaOrdnerHash.ps1'
if (-not (Test-Path -LiteralPath $ordnerHashWeg)) {
    Write-Host "ABBRUCH: NakamaOrdnerHash.ps1 fehlt neben diesem Skript ($ordnerHashWeg)." -ForegroundColor Red
    exit 1
}
. $ordnerHashWeg

function Loese-Pfad([string]$relativ) { Join-Path $wurzel ($relativ -replace '/', '\') }

function Schreibe-Ergebnis($stand) {
    $stand | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ergebnisWeg -Encoding UTF8
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
    if (-not $Pruefen -and -not $Rueckweg) {
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

function Ziel-Pfad($artefakt) {
    if ($artefakt.art -eq 'vst3') {
        $ident = Ident-Von $artefakt.ziel_id
        if ($null -eq $ident) { Abbruch "Ziel '$($artefakt.ziel_id)' steht nicht in der Identitaetsdatei." }
        # Der BUNDLE-ORDNER, nicht die Datei darin (Vertrag §2.1).
        return Join-Path ($manifest.ziele.vst3_verzeichnis -replace '/', '\') $ident.bundle
    }
    return Join-Path ($manifest.ziele.broker_verzeichnis -replace '/', '\') $artefakt.name
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
    $fehlend = @()
    $lauf = $pfad
    while ($lauf -and -not (Test-Path -LiteralPath $lauf)) {
        $fehlend = @($lauf) + $fehlend
        $eltern = Split-Path -Parent $lauf
        if ($eltern -eq $lauf -or [string]::IsNullOrEmpty($eltern)) { break }
        $lauf = $eltern
    }
    foreach ($d in $fehlend) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
    return $fehlend
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
    #
    # ZWEI HASHARTEN, EIN NACHSCHLAGEN (Vertrag §5.1, seit dem Umzug auf den
    # Ordner): die zwei historischen Eintraege wurden gegen die innere
    # BINAERDATEI eingefroren; neue Eintraege werden gegen den ORDNER
    # gebildet. Der Rueckweg schlaegt darum beide Zahlen nach - sonst faende
    # er den 16.08.-Stand nie wieder und verlangte bei jedem Rueckweg
    # -Erzwingen, also genau die Ermuedung, die einen Riegel wertlos macht.
    $bekanntOrdner = @{}
    $bekanntInnen  = @{}
    foreach ($b in $manifest.rueckweg.bekannte_staende) {
        if ($b.hash_art -eq 'ordner')           { $bekanntOrdner[$b.sha256] = $b }
        elseif ($b.hash_art -eq 'datei-innen')  { $bekanntInnen[$b.sha256]  = $b }
        else { Abbruch "bekannter Stand $($b.sha256) hat kein gueltiges hash_art ('$($b.hash_art)')." }
    }

    $warnungen = @()
    foreach ($e in $letzte.eintraege) {
        if (-not $e.gesichert) { continue }           # war vorher nicht da
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

    $getan = @()
    foreach ($e in $letzte.eintraege) {
        $art = if ($e.PSObject.Properties.Name -contains 'art' -and $e.art) { $e.art } else { 'broker' }
        if ($Pruefen) { Write-Host "  wuerde zurueckstellen: $($e.ziel)"; continue }
        if ($e.gesichert) {
            Kopiere-Stand (Loese-Pfad $e.gesichert) $e.ziel $art
            $getan += "wiederhergestellt: $($e.ziel)"
        }
        elseif (Entferne-Stand $e.ziel $art) {
            # Vorher gab es den Stand nicht — der Rueckweg ist das Entfernen.
            # Beim Bundle der GANZE Ordner: eine Datei zu loeschen und das
            # Gehaeuse stehenzulassen war Befund T2-5 (d).
            $getan += "entfernt: $($e.ziel)"
        }

        # Und die Verzeichnisse, die der Installationslauf SELBST angelegt
        # hat - von innen nach aussen, und nur solange sie leer sind. Zwei
        # Bedingungen, keine entbehrlich: "selbst angelegt" schuetzt
        # vorgefundene Ordner, "leer" schuetzt fremde Dateien, die inzwischen
        # dort liegen. Der Rueckweg stellt den Vorzustand her, auch wenn der
        # Vorzustand "kein Ordner" war.
        if ($e.PSObject.Properties.Name -contains 'erzeugte_ordner' -and $e.erzeugte_ordner) {
            $rueckwaerts = @($e.erzeugte_ordner)
            [array]::Reverse($rueckwaerts)
            foreach ($d in $rueckwaerts) {
                if (-not (Test-Path -LiteralPath $d -PathType Container)) { continue }
                if (@(Get-ChildItem -LiteralPath $d -Force).Count -gt 0) {
                    Write-Host "  bleibt (nicht leer): $d" -ForegroundColor DarkGray
                    continue
                }
                Remove-Item -LiteralPath $d -Force
                $getan += "Ordner entfernt: $d"
            }
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
    $was    = if ($a.art -eq 'vst3') { 'Bundle-Ordner' } else { 'Quelldatei' }
    $typ    = if ($a.art -eq 'vst3') { 'Container' } else { 'Leaf' }
    if (-not (Test-Path -LiteralPath $quelle -PathType $typ)) { Abbruch "$name : $was fehlt ($($a.quelle))." }
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

# Gegenpfad: erst sichern, dann tauschen.
$sicherungsOrdner = Join-Path $PSScriptRoot 'rueckweg'
if (-not (Test-Path -LiteralPath $sicherungsOrdner)) {
    New-Item -ItemType Directory -Path $sicherungsOrdner -Force | Out-Null
}
$stempel = [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')

$eintraege = @()
foreach ($p in $plan) {
    $art    = $p.Artefakt.art
    $vorher = Hash-Von $p.Ziel $art
    # Die innere Binaerdatei zusaetzlich: NUR damit der Rueckweg die zwei
    # historischen `bekannte_staende`-Eintraege wiederfindet, die gegen sie
    # eingefroren wurden (Vertrag §5.1).
    $vorherInnen = $null
    $gesichert   = $null
    if ($null -ne $vorher) {
        if ($art -eq 'vst3') { $vorherInnen = Datei-Hash (Innen-Pfad $p.Artefakt $p.Ziel) }
        $kennung = if ($art -eq 'vst3') { $p.Artefakt.ziel_id } else { 'broker' }
        $endung  = if ($art -eq 'vst3') { '.bundle' } else { '.bak' }
        $sicherung = Join-Path $sicherungsOrdner "$stempel-$kennung$endung"
        Kopiere-Stand $p.Ziel $sicherung $art
        # Relativ speichern: die Ergebnisdatei soll einen Workspace-Umzug ueberleben.
        $gesichert = 'eq-copilot/install/rueckweg/' + (Split-Path -Leaf $sicherung)
    }

    # Nur die Verzeichnisse merken, die dieser Lauf WIRKLICH anlegt. Sie sind
    # die einzigen, die der Rueckweg entfernen darf.
    $erzeugte = @(Erzeuge-Ordnerkette (Split-Path -Parent $p.Ziel))

    # Ersetzen, nicht mischen: Kopiere-Stand entfernt einen vorhandenen Stand
    # zuerst. `Copy-Item -Recurse` auf einen bestehenden Ordner fuehrte sonst
    # zusammen, und eine Datei, die der neue Bau nicht mehr enthaelt,
    # ueberlebte still (Befund T2-5 (b)).
    Kopiere-Stand $p.Quelle $p.Ziel $art

    $nachher = Hash-Von $p.Ziel $art
    if ($nachher -ne $p.Artefakt.sha256) {
        Abbruch "$($p.Name): installierter Stand stimmt nicht ($nachher). Der Rueckweg liegt in $gesichert."
    }
    Write-Host "  ok      $($p.Name) installiert  [$($p.Ziel)]"

    $eintraege += [pscustomobject]@{
        ziel_id             = $p.Artefakt.ziel_id
        art                 = $art
        name                = $p.Name
        ziel                = $p.Ziel
        sha256              = $nachher
        vorher_sha256       = $vorher
        vorher_sha256_innen = $vorherInnen
        gesichert           = $gesichert
        erzeugte_ordner     = $erzeugte
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
