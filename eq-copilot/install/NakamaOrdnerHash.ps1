<#
.SYNOPSIS
    Ordner-Hash v1 - die PowerShell-Haelfte. Vertrag:
    eq-copilot/schemas/installer/nakama-installer-v1.md §2.1.

.DESCRIPTION
    Ein VST3-Bundle ist unter Windows ein ORDNER. Seit der S9-Nacharbeit
    (T2-Befund T2-5, 23.08.2026) ist der Ordner die Auslieferungseinheit -
    beim Hashen, Sichern, Tauschen, Pruefen und beim Rueckweg.

    Ein SHA-256 ueber einen Ordner ist keine Selbstverstaendlichkeit; er
    braucht eine Definition, an die sich ZWEI Implementierungen halten:
    Python schreibt ihn (`pruefe_installer_manifest.py --hashen`), PowerShell
    prueft ihn (Install-Nakama.ps1). Diese Datei ist die eine PowerShell-
    Quelle dafuer - dot-gesourct vom Installer UND direkt gerufen von der
    Kreuzprobe des Kanon-Beins A17. Zwei Aufrufer, eine Funktion: waere sie
    im Installer eingebettet, koennte die Kreuzprobe nur eine Kopie messen.

    DIE FORM (woertlich wie im Vertrag §2.1):

      Fuer jede DATEI unterhalb des Ordners, rekursiv (Verzeichnisse selbst
      zaehlen nicht):
          relpfad := Pfad relativ zum Ordner, Trennzeichen '/', kein './'
          zeile   := <SHA-256 der Datei, 64 Hex GROSS> <Leerzeichen> <relpfad> <LF>
      Alle Zeilen aufsteigend nach relpfad sortiert, aneinandergehaengt,
      UTF-8; Ordner-Hash := SHA-256 darueber, 64 Hex GROSS.

    ZWEI ABBRUCHGRUENDE, beide bewusst laut statt still verschieden:

      1. Nicht-ASCII im relativen Pfad. PowerShell sortiert ordinal nach
         UTF-16-Code-Einheiten, Python byteweise nach UTF-8; oberhalb der BMP
         laufen diese Ordnungen auseinander. Der Bereich wird so weit
         eingeengt, dass beide Seiten NICHT uneins sein koennen.
      2. Ein Reparse-Punkt (Junction/Symlink) im Ordner. `-Recurse` steigt in
         ein Verzeichnis-Reparse nicht hinab, Pythons `rglob` schon -
         dieselbe Definition ergaebe zwei Hashes.

    Versteckte Dateien zaehlen mit (`-Force`): "unsichtbar" darf nicht
    "nicht ausgeliefert" heissen.
#>

# Bewusst KEIN Set-StrictMode hier: diese Datei wird dot-gesourct, und
# Set-StrictMode wirkt dann im Bereich des AUFRUFERS. Ein Modul, das die
# Semantik seines Aufrufers aendert, ist ein Seiteneffekt, den niemand an der
# Aufrufzeile sieht.

function Test-NakamaAsciiPfad {
    param([Parameter(Mandatory)][string]$Relpfad)
    foreach ($z in $Relpfad.ToCharArray()) {
        $c = [int][char]$z
        if ($c -lt 0x20 -or $c -gt 0x7E) { return $false }
    }
    return $true
}

<# Ordner-Hash v1 ueber $Ordner. Wirft, wenn der Ordner fehlt, ein Pfad
   nicht ASCII ist oder ein Reparse-Punkt darin liegt. #>
function Get-NakamaOrdnerHash {
    param([Parameter(Mandatory)][string]$Ordner)

    $voll = (Resolve-Path -LiteralPath $Ordner).Path
    if (-not (Test-Path -LiteralPath $voll -PathType Container)) {
        throw "Ordner-Hash v1: '$Ordner' ist kein Verzeichnis."
    }
    $praefix = $voll.TrimEnd('\') + '\'

    # Alles aufzaehlen, nicht nur Dateien: ein Verzeichnis-Reparse muss
    # AUFFALLEN, und dafuer muss es zuerst gesehen werden.
    $alle = @(Get-ChildItem -LiteralPath $voll -Recurse -Force -ErrorAction Stop)

    $zeilen = New-Object System.Collections.Generic.List[string]
    $pfade  = New-Object System.Collections.Generic.List[string]
    $nachHash = @{}

    foreach ($e in $alle) {
        if (($e.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Ordner-Hash v1: Reparse-Punkt in der Auslieferung ('$($e.FullName)'). Nicht vorgesehen - Python und PowerShell wuerden ihn verschieden behandeln."
        }
        if ($e.PSIsContainer) { continue }

        $rel = $e.FullName.Substring($praefix.Length).Replace('\', '/')
        if (-not (Test-NakamaAsciiPfad $rel)) {
            throw "Ordner-Hash v1: Pfad ist nicht ASCII ('$rel'). Nur ASCII sortiert in beiden Implementierungen gleich."
        }
        $pfade.Add($rel)
        $nachHash[$rel] = (Get-FileHash -LiteralPath $e.FullName -Algorithm SHA256).Hash.ToUpperInvariant()
    }

    # Ordinal, NICHT kulturabhaengig: 'B' (0x42) vor 'a' (0x61). Sort-Object
    # sortierte sonst nach Locale - und der Hash haenge an der Systemsprache.
    $sortiert = [string[]]$pfade
    [System.Array]::Sort($sortiert, [System.StringComparer]::Ordinal)

    foreach ($rel in $sortiert) { $zeilen.Add($nachHash[$rel] + ' ' + $rel + "`n") }

    $text  = [string]::Join('', $zeilen.ToArray())
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($text)
    $sha   = [System.Security.Cryptography.SHA256]::Create()
    try {
        return [System.BitConverter]::ToString($sha.ComputeHash($bytes)).Replace('-', '').ToUpperInvariant()
    }
    finally { $sha.Dispose() }
}

<# Der Hash EINES Artefakts: `vst3` ist ein Ordner, `broker` eine Datei.
   Gibt $null zurueck, wenn an dem Pfad nichts liegt - "nicht installiert"
   ist eine Antwort, kein Fehler. #>
function Get-NakamaArtefaktHash {
    param(
        [Parameter(Mandatory)][string]$Pfad,
        [Parameter(Mandatory)][ValidateSet('vst3', 'broker')][string]$Art
    )
    if ($Art -eq 'vst3') {
        if (-not (Test-Path -LiteralPath $Pfad -PathType Container)) { return $null }
        return Get-NakamaOrdnerHash -Ordner $Pfad
    }
    if (-not (Test-Path -LiteralPath $Pfad -PathType Leaf)) { return $null }
    return (Get-FileHash -LiteralPath $Pfad -Algorithm SHA256).Hash.ToUpperInvariant()
}

# Direktaufruf fuer die Kreuzprobe von A17:
#   pwsh -File NakamaOrdnerHash.ps1 <ordner>   -> schreibt den Hash auf stdout
if ($MyInvocation.InvocationName -ne '.' -and $args.Count -ge 1) {
    Write-Output (Get-NakamaOrdnerHash -Ordner $args[0])
}
