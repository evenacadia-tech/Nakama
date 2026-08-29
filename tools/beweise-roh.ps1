#Requires -Version 7.0
<#
.SYNOPSIS
    Atomare Reservierung der Kanon-Rohausgabe-Datei fuer tools/beweise.ps1.

.DESCRIPTION
    NAK-96 hat die rohen stdout/stderr aus dem Manifest in eine eigene Datei je
    Lauf ausgelagert (docs/beweise/roh/<TICKET>-<sha7>[-dirty][-N].md). Der
    Grundsatz dabei: eine bestehende Rohausgabe wird NIE ueberschrieben - ein
    Beweis, den ein spaeterer Lauf still ersetzt, ist kein Beweis.

    Die erste Fassung suchte den freien Namen mit `while (Test-Path) { N++ }`
    und schrieb ihn erst Minuten spaeter mit `Set-Content`. Zwischen Pruefung
    und Schreiben passt ein zweiter Runner: beide sehen denselben Namen frei,
    beide waehlen ihn, der zweite ersetzt die Datei des ersten still. Der
    Kommentar behauptete damit mehr, als die Implementierung hielt
    (Pruefliste E, Pruefer-Befund P1 vom 29.08.2026).

    Diese Datei haelt die Reservierung, die den Namen im selben
    Betriebssystem-Aufruf prueft UND belegt: [IO.File]::Open(...CreateNew...)
    legt die Datei an oder wirft, wenn sie schon existiert. Der Aufrufer haelt
    das offene Handle bis zum Schreiben und laesst FileShare::None niemanden
    dazwischen.

    Eigene Datei statt Funktion im Runner, damit die Rennprobe sie in einem
    Testprozess laden kann, OHNE den 25-Minuten-Kanon zu starten:
    `. tools/beweise-roh.ps1` definiert die Funktion und tut sonst nichts.

.NOTES
    Wird von tools/beweise.ps1 per Dot-Sourcing geladen.
#>

function Reserviere-Rohdatei {
    <#
    .SYNOPSIS
        Belegt den ersten freien Rohausgabe-Namen atomar und liefert Pfad und
        offenes Schreib-Handle zurueck.

    .DESCRIPTION
        Namensschema wie seit NAK-96: <Basisname>.md, danach <Basisname>-2.md,
        <Basisname>-3.md ... Der erste Name, den CreateNew anlegen kann, gewinnt;
        wer die IOException bekommt, nimmt den naechsten.

        Der Aufrufer MUSS den zurueckgegebenen Strom schliessen. Bricht er
        vorher ab, bleibt eine 0-Byte-Datei liegen - das ist der bewusste Preis
        der Atomaritaet: lieber eine sichtbar leere Reservierung als ein
        stillschweigend ueberschriebener Beweis.

    .PARAMETER Verzeichnis
        Zielverzeichnis; muss existieren (fehlt es, wirft die Funktion sofort
        weiter, statt 1000 leere Runden zu drehen).

    .PARAMETER Basisname
        Dateiname ohne Endung und ohne Zaehlsuffix, z. B. NAK-96-afc0978-dirty.

    .PARAMETER MaxVersuche
        Obergrenze der Suffixsuche. Danach Abbruch mit Fehler - eine
        Reservierungsschleife darf nie endlos laufen.
    #>
    param(
        [Parameter(Mandatory)][string] $Verzeichnis,
        [Parameter(Mandatory)][string] $Basisname,
        [int] $MaxVersuche = 1000
    )

    for ($n = 1; $n -le $MaxVersuche; $n++) {
        $pfad = if ($n -eq 1) {
            Join-Path $Verzeichnis ($Basisname + '.md')
        }
        else {
            Join-Path $Verzeichnis ('{0}-{1}.md' -f $Basisname, $n)
        }

        try {
            # Pruefen und Belegen in EINEM Aufruf: CreateNew ist die atomare
            # Zusage des Dateisystems, dass genau dieser Prozess den Namen hat.
            $strom = [IO.File]::Open($pfad, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
            return [pscustomobject]@{ Pfad = $pfad; Strom = $strom; Versuche = $n }
        }
        # Reihenfolge zaehlt: beide erben von IOException und wuerden sonst als
        # "Name belegt" gelesen - ein fehlendes Verzeichnis muss aber sofort
        # auffliegen, nicht als 1000 belegte Namen erscheinen.
        catch [IO.DirectoryNotFoundException] { throw }
        catch [IO.PathTooLongException]       { throw }
        catch [IO.IOException]                { continue }
    }

    # Klammern sind Pflicht: -f bindet staerker als +, sonst formatiert die
    # Zeile den ZWEITEN Teilstring (der keine Platzhalter hat) und der erste
    # bleibt mit {0}{1}{2} stehen - gemessen bei der Wachenprobe.
    throw (("Rohausgabe nicht reservierbar: '{0}' in '{1}' ist auch nach {2} Versuchen belegt. " +
            "Kein Beweis wird ueberschrieben - der Lauf bricht ab.") -f $Basisname, $Verzeichnis, $MaxVersuche)
}
