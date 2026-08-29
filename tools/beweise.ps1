#Requires -Version 7.0
<#
.SYNOPSIS
    Nakama-Beweis-Runner - faehrt den kompletten Beweis-Kanon, schreibt Urteil
    und Uebersicht in ein Manifest unter docs/beweise/ und die ROHE Ausgabe
    daneben nach docs/beweise/roh/.

.DESCRIPTION
    Ersatz fuer die bewusst nicht gebaute CI (docs/bauaufteilung-sonden.md Abschnitt 1.1):
    eine Maschine, ein Entwickler, ein JUCE-Windows-Build. Ein Befehl statt zehn
    Handgriffe - das ist der Unterschied zwischen Beweisstandard und
    Beweisvorsatz.

    Der Runner fasst NICHTS zusammen: stdout und stderr jedes Laufs landen
    unveraendert auf der Platte. Seit NAK-96 in ZWEI Dateien (vorher wuchs jedes
    Manifest um ~3000 Zeilen Rohtext je Lauf und wurde unlesbar):

      Manifest (-Ziel)  Ueberschrift, die Zeile "**Lauf:** ... **Urteil:** ...
                        **Exitcode:** ... **Rohausgabe:** <Verweis>", der Kopf
                        "woran gemessen wurde" und die Uebersichtstabelle. Die
                        Uebersicht verlinkt je Bein in die Rohausgabe.
      Rohausgabe        docs/beweise/roh/<TICKET>-<sha7>.md - derselbe Kopf plus
                        Baustand, vollstaendige stdout/stderr jedes Beins und
                        das Bauprotokoll. Bei unbestaetigtem Arbeitsbaum traegt
                        der Name zusaetzlich `-dirty`, bei einem zweiten Lauf
                        auf demselben Stand ein Zaehlsuffix. Bestehende
                        Rohausgaben werden NIE ueberschrieben.

    Der Wortlaut der Lauf-Zeile ist fest: tools/plan/planstand.py liest daraus
    die Kanon-Zahl zurueck (Regex KANON). Nur anhaengen, nie umformulieren.

    Er misst ausserdem den BAUSTAND - sind die Pruefbinaries aelter als die
    Quellen, verweigert er die Beglaubigung (Exitcode 4), statt eine veraltete
    Messung als Basislinie auszugeben.

    Lesende git-Aufrufe laufen mit --no-optional-locks (NAK-96): sonst frischt
    git den Index auf, legt .git/index.lock an, und ein unter Last
    abgeschossener Aufruf laesst die Sperre liegen.

    Der Kanon waechst mit dem Plan: geplante Pruefbinaries (ab P0/P1/P2/P6)
    stehen bereits in der Tabelle. Solange sie fehlen, sind sie eine neutrale
    Zeile "geplant"; sobald sie gebaut sind, laufen sie automatisch mit und
    sind ab dann Pflicht.

.PARAMETER Ziel
    Zieldatei des Manifests (Lesetext). Vorgabe: docs/beweise/lauf-<zeit>.md
    Die Rohausgabe geht immer nach docs/beweise/roh/ und wird aus dem
    Dateinamen des Ziels abgeleitet.

.PARAMETER Anhaengen
    Haengt den Lesetext des Laufs an ein bestehendes Ticket-Manifest an, statt
    es zu ersetzen. Die Rohausgabe ist in jedem Fall eine eigene neue Datei.

.PARAMETER Bauen
    Baut die Kanon-Ziele vorher neu (Release), damit der Lauf den aktuellen
    Quellstand beweist.

.PARAMETER Titel
    Ueberschrift des Manifests bzw. des angehaengten Abschnitts.

.EXAMPLE
    pwsh -File tools/beweise.ps1 -Ziel docs/beweise/S0-basislinie.md -Titel 'S0 | Basislinie'

.EXAMPLE
    pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-004.md -Anhaengen

.NOTES
    Exitcodes
      0  alles gruen
      2  mindestens ein Kanon-Lauf rot
      3  Voraussetzung fehlt (nicht gebaut, keine Fixtures, kein cargo)
      4  Laeufe gruen, aber Binaries aelter als die Quellen (nicht beglaubigt)
    Reihenfolge der Beurteilung: 2 vor 3 vor 4.
#>

[CmdletBinding()]
param(
    [string] $Ziel,
    [switch] $Anhaengen,
    [switch] $Bauen,
    [string] $Titel = 'Kanon-Lauf'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# Native Befehle duerfen mit Exit != 0 zurueckkommen, ohne das Skript zu toeten -
# genau das ist ja der Messwert (PS 7.4+ wuerfe sonst bei ErrorActionPreference=Stop).
$PSNativeCommandUseErrorActionPreference = $false

$Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Beginn = Get-Date

# ---------------------------------------------------------------- Hilfsmittel

function Argument-Quoten {
    <# Start-Process -ArgumentList klebt die Elemente UNQUOTIERT mit Leerzeichen
       zusammen (anders als ProcessStartInfo.ArgumentList in .NET). Ohne diese
       Quotierung zerfiele jeder Pfad mit Leerzeichen still in zwei Argumente -
       gemessen an "git log --format=%h %s", das dabei Exit 128 lieferte. #>
    param([string] $Argument)
    if ($null -eq $Argument -or $Argument -eq '') { return '""' }
    if ($Argument -notmatch '[\s"]') { return $Argument }
    # Windows-Kommandozeilenregeln: Backslashes vor einem " (und am Ende) verdoppeln.
    $s = [regex]::Replace($Argument, '(\\*)"', '$1$1\"')
    $s = [regex]::Replace($s, '(\\+)$', '$1$1')
    return '"' + $s + '"'
}

function Fuehre-Aus {
    <# Fuehrt ein Programm aus und liefert Exitcode, stdout, stderr und Dauer.
       stdout/stderr gehen getrennt in Temp-Dateien: kein PowerShell-Fehlerstrom
       dazwischen, keine ErrorRecords, keine Verstuemmelung. #>
    param(
        [Parameter(Mandatory)][string] $Datei,
        [string[]] $Argumente = @(),
        [string] $Arbeitsverzeichnis = $Wurzel
    )

    $ausDatei = [IO.Path]::GetTempFileName()
    $errDatei = [IO.Path]::GetTempFileName()
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $code = -1
    $fehlertext = ''

    try {
        $start = @{
            FilePath               = $Datei
            NoNewWindow            = $true
            Wait                   = $true
            PassThru               = $true
            RedirectStandardOutput = $ausDatei
            RedirectStandardError  = $errDatei
            WorkingDirectory       = $Arbeitsverzeichnis
        }
        if ($Argumente.Count -gt 0) { $start.ArgumentList = @($Argumente | ForEach-Object { Argument-Quoten $_ }) }

        $prozess = Start-Process @start
        if ($null -ne $prozess) {
            $prozess.WaitForExit()
            $code = $prozess.ExitCode
        }
    }
    catch {
        $fehlertext = $_.Exception.Message
    }

    $uhr.Stop()
    $aus = ''
    $err = ''
    if (Test-Path -LiteralPath $ausDatei) { $aus = (Get-Content -LiteralPath $ausDatei -Raw -Encoding utf8) ?? '' }
    if (Test-Path -LiteralPath $errDatei) { $err = (Get-Content -LiteralPath $errDatei -Raw -Encoding utf8) ?? '' }
    if ($fehlertext) { $err = ($err + "`n[Start-Process] " + $fehlertext).Trim() }
    Remove-Item -LiteralPath $ausDatei, $errDatei -Force -ErrorAction SilentlyContinue

    [pscustomobject]@{
        ExitCode = $code
        StdOut   = $aus
        StdErr   = $err
        Sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 2)
    }
}

function Einzeilig {
    <# Erste Ausgabezeile eines Programms - fuer Versionsabfragen. #>
    param([string] $Datei, [string[]] $Argumente = @())
    if (-not (Get-Command $Datei -ErrorAction SilentlyContinue) -and -not (Test-Path -LiteralPath $Datei)) {
        return 'nicht gefunden'
    }
    $r = Fuehre-Aus -Datei $Datei -Argumente $Argumente
    $text = ($r.StdOut + "`n" + $r.StdErr).Trim()
    if (-not $text) { return 'nicht ermittelbar' }
    return ($text -split "`r?`n")[0].Trim()
}

function Git-Wert {
    <# Nur lesende Abfragen. `--no-optional-locks` steht VOR `-C`, weil git
       globale Schalter vor dem Unterbefehl erwartet; ohne ihn frischt
       `git status` den Index auf und legt dafuer `.git/index.lock` an -
       wird der Aufruf unter Last abgeschossen, bleibt die Sperre liegen
       und blockiert jedes spaetere git (NAK-96, dreimal am 29.08.). #>
    param([string[]] $Argumente)
    $r = Fuehre-Aus -Datei 'git' -Argumente (@('--no-optional-locks', '-C', $Wurzel) + $Argumente)
    if ($r.ExitCode -ne 0) { return 'nicht ermittelbar' }
    return $r.StdOut.Trim()
}

function Zellentext {
    <# Markdown-Tabellenzellen vertragen kein rohes | und keine Zeilenumbrueche. #>
    param([string] $Text)
    if ($null -eq $Text) { return '' }
    return ($Text -replace '\|', '\|' -replace '\r?\n', ' ').Trim()
}

function Zaun {
    <# Waehlt einen Codezaun, der laenger ist als jede Zaunfolge im Inhalt -
       sonst zerreisst eine Testausgabe mit dreifachem Backtick das Manifest. #>
    param([string] $Inhalt)
    $laenge = 3
    foreach ($treffer in [regex]::Matches(($Inhalt ?? ''), '`{3,}')) {
        if ($treffer.Value.Length -ge $laenge) { $laenge = $treffer.Value.Length + 1 }
    }
    return ('`' * $laenge)
}

function Block {
    <# Rohausgabe als Codeblock - unveraendert, nur eingezaeunt. #>
    param([string] $Inhalt, [string] $Sprache = 'text')
    if ([string]::IsNullOrWhiteSpace($Inhalt)) { return '_(leer)_' }
    $zaun = Zaun $Inhalt
    return ($zaun + $Sprache + "`n" + $Inhalt.TrimEnd() + "`n" + $zaun)
}

function Dauertext {
    <# Eine Schreibweise fuer Konsole und Manifest (Kultur der Maschine). #>
    param([double] $Sekunden)
    return ('{0:N2} s' -f $Sekunden)
}

function Pruefbinaer {
    param([string] $Name)
    return (Join-Path $Wurzel ("eq-copilot\build\plugin\{0}_artefacts\Release\{0}.exe" -f $Name))
}

function RelativZurWurzel {
    param([string] $Pfad)
    if ($Pfad.StartsWith($Wurzel, [StringComparison]::OrdinalIgnoreCase)) {
        return $Pfad.Substring($Wurzel.Length).TrimStart('\', '/')
    }
    return $Pfad
}

# ------------------------------------------------------------------ Der Kanon

$fixtureVerzeichnis = Join-Path $Wurzel 'eq-copilot\fixtures'

<# Stillgelegt? Unter `Set-StrictMode -Version Latest` wirft `$e.Stillgelegt`
   bei jedem Eintrag, der das Feld gar nicht hat - also bei allen ausser dem
   einen. Die Frage nach der ANWESENHEIT einer Eigenschaft gehoert deshalb an
   PSObject, nicht an den Punktzugriff. (Gemessen am ersten Lauf nach der
   Aenderung, S9b/SONDE-007c: "The property 'Stillgelegt' cannot be found on
   this object.") #>
function Ist-Stillgelegt($eintrag) {
    return $null -ne $eintrag.PSObject.Properties['Stillgelegt']
}

$kanon = @(
    [pscustomobject]@{
        Kuerzel    = 'A1'
        Name       = 'EqCopNullTest'
        Art        = 'plugin'
        Argumente  = @()
        AbPhase    = 'jetzt'
        Behauptung = 'Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert.'
    }
    [pscustomobject]@{
        Kuerzel    = 'A2'
        Name       = 'EqCopGoldenTest'
        Art        = 'plugin'
        Argumente  = @($fixtureVerzeichnis)
        AbPhase    = 'jetzt'
        Behauptung = 'AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel).'
    }
    [pscustomobject]@{
        Kuerzel    = 'A3'
        Name       = 'EqCopMarkierungTest'
        Art        = 'plugin'
        Argumente  = @()
        AbPhase    = 'jetzt'
        Behauptung = 'Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport).'
    }
    [pscustomobject]@{
        Kuerzel    = 'A4'
        Name       = 'broker'
        Art        = 'cargo'
        Argumente  = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never')
        AbPhase    = 'jetzt'
        Behauptung = 'Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server).'
    }
    [pscustomobject]@{
        Kuerzel    = 'A4b'
        Name       = 'EqCopPipeClientTest'
        Art        = 'plugin'
        Argumente  = @()
        AbPhase    = 'jetzt'
        Behauptung = 'C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher.'
    }
    # --- Python-Beine des v3-Vertrags ---------------------------------------
    #
    # T2-Befund vom 21.08. (SONDE-005a, Runde 1): vier von zwoelf Behauptungen
    # des Manifests liefen NUR von Hand - das Referenzbein und die drei
    # Bytegleichheits-Riegel. Ausgerechnet die Bytegleichheit ist der Riegel,
    # der gegen `core.autocrlf` auf dem Zweitrechner errichtet wurde; ein
    # Riegel, den niemand faehrt, ist keiner.
    [pscustomobject]@{ Kuerzel='A5'; Name='pruefe_v3_vertrag.py';   Art='python'; Argumente=@('--abdeckung'); AbPhase='jetzt'; Behauptung='Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture.' }
    [pscustomobject]@{ Kuerzel='A6'; Name='erzeuge_bandgitter.py';  Art='python'; Argumente=@('--pruefen');   AbPhase='jetzt'; Behauptung='Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition.' }
    [pscustomobject]@{ Kuerzel='A7'; Name='erzeuge_quantisierung.py'; Art='python'; Argumente=@('--pruefen'); AbPhase='jetzt'; Behauptung='Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren.' }
    [pscustomobject]@{ Kuerzel='A8'; Name='erzeuge_v3_fixtures.py'; Art='python'; Argumente=@('--pruefen');   AbPhase='jetzt'; Behauptung='Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei.' }

    # --- SONDE-005b: die zweite Gate-Haelfte aus §65 --------------------------
    #
    # "Codegen-Drift ist 0" ist nur dann eine Aussage, wenn sie GEFAHREN wird.
    # A9 prueft zusaetzlich, dass flatc, C++-Header und Rust-Crate dieselbe
    # Version tragen - der erzeugte Code ruft in diese Laufzeit.
    [pscustomobject]@{ Kuerzel='A9';  Name='pruefe_flatc_drift.py';  Art='python'; Argumente=@();            AbPhase='jetzt'; Behauptung='Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID.' }
    [pscustomobject]@{ Kuerzel='A10'; Name='erzeuge_fb_fixtures.py'; Art='python'; Argumente=@('--pruefen'); AbPhase='jetzt'; Behauptung='Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei.' }

    # --- v2-Vertraege (Kontext-Inventur 21.08.) --------------------------------
    #
    # eq-snapshot.schema.json war seit dem 15.08. kein gueltiges JSON, und kein
    # Bein hat es gemerkt: keines las die fuenf v2-Schemas maschinell. Ein
    # Vertrag, den keine Maschine liest, ist keiner - A11 liest sie.
    [pscustomobject]@{ Kuerzel='A11'; Name='pruefe_v2_schemas.py';   Art='python'; Argumente=@();            AbPhase='jetzt'; Behauptung='Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren.' }

    # --- SONDE-006: State-Vertrag und state_hash --------------------------------
    #
    # Der Parameterbestand (109 IDs) und der RFC-8785-Korpus sind handgeschrieben;
    # A12 prueft den Vertrag, laesst die Python-Referenz (rfc8785) jede
    # Hand-Erwartung bestaetigen und haelt Korpus + MANIFEST bytegleich.
    # B2 (C++) und das Rust-Bein in A4 messen gegen dasselbe MANIFEST.
    [pscustomobject]@{ Kuerzel='A12'; Name='erzeuge_state_fixtures.py'; Art='python'; Argumente=@('--pruefen'); AbPhase='jetzt'; Behauptung='Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung.' }

    # --- SONDE-004 (S4): Capabilityreport aus FL-Termin A + B --------------------
    #
    # §53.6: ein Capabilitybit wird nur nach seinem Golden gesetzt. A13 bindet
    # jedes der zehn Bits in identity/host-capabilities-fl-v1.json an das
    # Rohfeld der Messung (docs/beweise/termin-a/, termin-b/) und an die
    # Vertragsform aus schemas/v3 ($defs/capabilities).
    [pscustomobject]@{ Kuerzel='A13'; Name='pruefe_host_capabilities.py'; Art='python'; Argumente=@(); AbPhase='jetzt'; Behauptung='Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback.' }
    # S8/SONDE-007a: K1 (Praeprozessor) und K2 (CMake) reden ueber die
    # BAUBESCHREIBUNG und laufen beim Bauen mit. A14 ist der einzige Riegel, der
    # das ARTEFAKT misst - und der einzige, der ein Stringliteral faende, das
    # kein Makro ist. Er traegt seine eigene Gegenprobe: derselbe Scanner muss
    # die Werte im gebauten Bundle FINDEN, sonst ist sein Schweigen wertlos.
    [pscustomobject]@{ Kuerzel='A14'; Name='pruefe_kern_identitaetsfrei.py'; Art='python'; Argumente=@(); AbPhase='jetzt'; Behauptung='Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle.' }

    # S9/SONDE-007b: das Grundgesetz gilt fuer JEDES Bundle, das Audio traegt.
    # Zwei Beine, weil die Produktklasse ein Uebersetzungsschalter ist - ein
    # Lauf kann nicht beide messen. Probeeq ist heute ebenfalls Passthrough;
    # kommt seine DSP in P6, MUSS A16 umgeschrieben werden. Faellt es dann,
    # ist das die richtige Reaktion.
    # S9/SONDE-007b Abschnitt 3: das Installer-Manifest. A17 misst dieselbe
    # Regel wie NAK-52 auf der Bauseite - eine Identitaet, ein Ort - nur auf
    # der Auslieferungsseite: Quellpfade werden aus Ziel + Identitaetsdatei
    # NACHGERECHNET, Viercodes und CIDs duerfen nirgends stehen. Das Bein
    # traegt seine Gegenprobe im Lauf: jede Regel muss an einem absichtlich
    # verdorbenen Manifest FALLEN, sonst prueft sie nichts (S8-Lehre).
    [pscustomobject]@{ Kuerzel='A17'; Name='pruefe_installer_manifest.py'; Art='python'; Argumente=@(); AbPhase='jetzt'; Behauptung='Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab.' }

    # S9-Nacharbeit 23.08.2026: die ZWEITE Haelfte des Gegenpfades. S9 hatte
    # sie geschrieben, aber nie ausgefuehrt (T2 §5.9: "Weder installiert noch
    # zurueckgerollt") - beim ersten wirklichen Lauf fielen zwei echte Fehler
    # heraus, die dem Lesen entgangen waren. A18 faehrt das ECHTE
    # Install-Nakama.ps1 in einer Sandbox unter %TEMP%; es installiert nichts
    # und braucht keine Rechteerhoehung. Eine Zusage, die nie ausgefuehrt
    # wird, ist keine Zusage.
    [pscustomobject]@{ Kuerzel='A18'; Name='pruefe_installer_gegenpfad.py'; Art='python'; Argumente=@(); AbPhase='jetzt'; Behauptung='Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab.' }

    # S12-13/SONDE-009: Drift-Riegel des Bandgitter-Headers, gleiche Form wie der
    # flatc-Drift-Test aus SONDE-005b. Das C++-Bein B5 misst die WERTE gegen die
    # Fixture; dieses Bein misst zusaetzlich, dass die Datei reproduzierbar aus
    # dem Erzeuger kommt. Die Luecke, die es schliesst: ein von Hand gepflegter
    # Header mit zufaellig richtigen Zahlen sieht fuer B5 gruen aus - und der
    # naechste Generatorlauf ueberschreibt die Handarbeit still.
    [pscustomobject]@{ Kuerzel='A19'; Name='erzeuge_bandgitter_header.py'; Art='python'; Argumente=@('--pruefen'); AbPhase='jetzt'; Behauptung='BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter).' }

    # --- S14-15/SONDE-010: der v3-Nachrichtenweg -----------------------------
    #
    # Drei neue A-Beine plus B10. Warum `transport_fuzz` ein EIGENER Eintrag
    # ist, obwohl A4 (`cargo test`) es ohnehin mitfaehrt: der Gate-Text von
    # §65 nennt Fuzz namentlich, und ein namentlich geforderter Beweis, der
    # nur in einer Sammelzeile steckt, ist von aussen nicht als gefahren zu
    # erkennen. Die paar Sekunden Doppellauf sind der Preis fuer eine eigene
    # gruene Zeile im Manifest.
    [pscustomobject]@{ Kuerzel='A20'; Name='erzeuge_envelope_fixtures.py'; Art='python'; Argumente=@('--pruefen'); AbPhase='jetzt'; Behauptung='Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture.' }
    [pscustomobject]@{ Kuerzel='A21'; Name='transport_fuzz'; Art='cargo'; Argumente=@('test', '--manifest-path', 'broker/Cargo.toml', '--test', 'transport_fuzz', '--color', 'never'); AbPhase='jetzt'; Behauptung='v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut.' }
    [pscustomobject]@{ Kuerzel='A22'; Name='pruefe_ipc_last.py'; Art='python'; Argumente=@(); AbPhase='jetzt'; Behauptung='Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen.' }

    # ── STILLGELEGT 28.08.2026 (S9b/SONDE-007c) ────────────────────────────
    # Die Zeile bleibt STEHEN. Ein Runner, der ein Bein einfach loescht, meldet
    # eine kleinere Zahl und sagt nicht, warum - und "28 statt 29" ist von
    # aussen nicht von "eine Pruefung verschwunden" zu unterscheiden. Deshalb
    # traegt der Eintrag ab hier `Stillgelegt`: er laeuft nicht mehr, wird
    # nicht mehr gebaut, zaehlt weder als gruen noch als rot - und steht in
    # der Uebersichtstabelle jedes Manifests mit Datum und Grund.
    #
    # Grund: das gemessene Ziel NakamaSuna ist stillgelegt (Suna ist seit dem
    # 23.08.2026 in Probeeq aufgegangen; User-Entscheid 28.08.2026,
    # design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Ein
    # Passthrough-Beweis fuer ein Bundle, das niemand baut und niemand
    # ausliefert, beweist nichts.
    [pscustomobject]@{ Kuerzel='A15'; Name='EqCopSunaNullTest';   Art='plugin'; Argumente=@(); AbPhase='jetzt'; Stillgelegt='seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt.'; Behauptung='STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich.' }
    [pscustomobject]@{ Kuerzel='A16'; Name='EqCopProbeeqNullTest'; Art='plugin'; Argumente=@(); AbPhase='jetzt'; Behauptung='Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2.' }

    # --- geplant: laufen automatisch mit, sobald sie gebaut sind -------------
    [pscustomobject]@{ Kuerzel='B1'; Name='EqCopIdentityTest';       Art='plugin'; Argumente=@(); AbPhase='P0'; Behauptung='Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren.' }
    # B2 wurde in P1 gebaut (SONDE-006): Schema 2, Parameterbestand, Migration, state_hash.
    [pscustomobject]@{ Kuerzel='B2'; Name='EqCopStateMigrationTest'; Art='plugin'; Argumente=@(); AbPhase='P1'; Behauptung='State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust.' }
    # B3 wurde in P0 gebaut (SONDE-003, Bridge-Patch), nicht erst in P2 - die
    # Phase ist hier berichtigt, damit die Zeile nicht laenger "geplant" heisst,
    # waehrend das Ziel schon Pflicht ist.
    [pscustomobject]@{ Kuerzel='B3'; Name='EqCopHostContextTest';    Art='plugin'; Argumente=@(); AbPhase='P0'; Behauptung='Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen.' }
    # S3b: das Termin-B-Messgeraet beweist sich selbst, BEVOR der User damit misst.
    [pscustomobject]@{ Kuerzel='B3b'; Name='EqCopHostProbeTest';     Art='plugin'; Argumente=@(); AbPhase='P0'; Behauptung='Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen.' }
    # SONDE-005a: der v3-Vertrag. Das C++-Bein misst gegen dasselbe
    # handgeschriebene MANIFEST wie contract_cross_language auf der Rust-Seite.
    [pscustomobject]@{ Kuerzel='B3c'; Name='EqCopSchemaTest';         Art='plugin'; Argumente=@(); AbPhase='P1'; Behauptung='v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich.' }
    # S10-11/SONDE-008: B4 wird Pflicht. Zwei Ebenen in einem Bein - Queue und
    # Quarantaene pur (dort sind Ueberlauf, Wrap und Oversize deterministisch
    # herbeifuehrbar) UND derselbe Weg verdrahtet im echten Prozessor, dort an
    # AUDIO und am Allokationszaehler gemessen. Der Zaehler ist thread_local:
    # der Worker DARF allozieren, der Audiothread nicht - ein globaler Zaehler
    # koennte beide nicht auseinanderhalten und waere damit wertlos.
    [pscustomobject]@{ Kuerzel='B4'; Name='EqCopQueueStressTest';    Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene.' }
    # S10-11/SONDE-008: der EBU-Golden des Gate-Textes. Seine Referenz ist die
    # AUSGEBAUTE Rechnung (unbegrenzter Vektor + Zweitdurchgang) - eine gegen
    # sich selbst gerechnete Zahl waere eine Tautologie.
    [pscustomobject]@{ Kuerzel='B9'; Name='EqCopLoudnessGoldenTest'; Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt.' }
    # S12-13/SONDE-009 gebaut: B5 ist Pflicht. Zwei Ebenen in einem Bein - die
    # FeatureEngine pur (dort sind alle neun Grenzarten deterministisch
    # herbeifuehrbar) UND derselbe Weg im echten Prozessor, wo Abschnitt K die
    # Brueckenhaelfte des Zeitstempels faehrt, die bis hierher kein Bein hatte
    # (NAK-56, T2-Bericht zu SONDE-008 §8.9 Punkt 2).
    [pscustomobject]@{ Kuerzel='B5'; Name='EqCopAnalysisGoldenTest'; Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei.' }
    [pscustomobject]@{ Kuerzel='B6'; Name='EqCopDspGoldenTest';      Art='plugin'; Argumente=@(); AbPhase='P6'; Behauptung='Aktiver DSP-Kern liefert die eingefrorene Referenzantwort.' }
    [pscustomobject]@{ Kuerzel='B7'; Name='EqCopTransactionTest';    Art='plugin'; Argumente=@(); AbPhase='P6'; Behauptung='Apply/Revert ist transaktional - kein halber Zustand ueberlebt.' }
    # S9/SONDE-007b Abschnitt 3: die Lifecycle-Klassifikation (§53.5). Zwei
    # Ebenen in einem Bein - der Automat pur UND derselbe Automat verdrahtet im
    # echten Prozessor, dort an AUDIO gemessen: dieselbe Markierung, die A3
    # faerben laesst, bleibt hier stumm, solange nicht klassifiziert ist.
    # S14-15/SONDE-010: das C++-Bein des v3-Nachrichtenwegs. Zwei Ebenen in
    # EINEM Ziel, wie B4 und B8: Envelope/Pipetoken/Backpressure pur UND die
    # ECHTEN Clients gegen einen selbst gehosteten v3-Server auf einer
    # Testpipe. Ohne die zweite Ebene waere es ein huebscher Parser, der im
    # Produkt nichts verbindet.
    [pscustomobject]@{ Kuerzel='B10'; Name='EqCopIpcTest'; Art='plugin'; Argumente=@(); AbPhase='P2'; Behauptung='v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen.' }
    [pscustomobject]@{ Kuerzel='B8'; Name='EqCopLebenslaufTest';     Art='plugin'; Argumente=@(); AbPhase='P1'; Behauptung='Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor|pre|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main.' }
)

# Ziele, die nicht selbst im Kanon laufen, aber von einem Kanon-Lauf GEMESSEN
# werden. Ohne sie prueft der Test ein altes Artefakt und bleibt gruen, waehrend
# die Quelle laengst etwas anderes sagt (T2-Befund vom 20.08.).
$gemesseneZiele = @(
    [pscustomobject]@{
        Ziel   = 'EqCopilot_VST3'
        Marker = 'juce_add_plugin(EqCopilot'
        Wegen  = 'EqCopIdentityTest misst dessen moduleinfo.json; A14 nutzt das Binary als Gegenprobe'
    }
    [pscustomobject]@{
        Ziel   = 'NakamaKern'
        Marker = 'add_library(NakamaKern STATIC'
        Wegen  = 'A14 misst die gebaute NakamaKern.lib (S8/SONDE-007a)'
    }
    # S9/SONDE-007b: das Sondenbundle. Sein Marker ist der Aufruf der
    # gemeinsamen Zielfunktion - jedes Sondenziel entsteht aus EINER
    # Vorschrift, also ist der Aufruf die Zeile, die sich aendert, wenn sich
    # etwas aendert.
    #
    # S9b/SONDE-007c (28.08.2026): der Eintrag fuer 'NakamaSuna_VST3' ist hier
    # ENTFERNT, weil das Ziel stillgelegt ist. Ein Marker, der ins Leere
    # zeigte, waere harmlos-still (die Where-Object-Bedingung faende ihn
    # einfach nicht mehr); ein Marker, der noch traefe, liesse den Runner ein
    # Ziel bauen, das es nicht gibt, und der Bau braeche ab. Beides waere
    # falsch - der Grund gehoert in den Text, nicht in eine tote Zeile.
    # S14-15/SONDE-010: die C++-Haelfte des Lastbeins. Sie ist selbst KEIN
    # Kanon-Bein - A22 startet sie gegen den Rust-Probe-Broker -, muss aber
    # frisch gebaut sein, sonst misst A22 ein altes Artefakt.
    [pscustomobject]@{
        Ziel   = 'EqCopIpcLast'
        Marker = 'juce_add_console_app(EqCopIpcLast'
        Wegen  = 'A22 (pruefe_ipc_last.py) faehrt dieses Programm gegen den Rust-Listener'
    }
    [pscustomobject]@{
        Ziel   = 'NakamaProbeeq_VST3'
        Marker = 'nakama_sonde_ziel(NakamaProbeeq'
        Wegen  = 'EqCopIdentityTest misst dessen moduleinfo.json gegen die reservierte NkAc-CID'
    }
)

# ------------------------------------------------------ Optional: vorher bauen

$cmakeBefehl = $null
$bauProtokoll = @()

function Finde-CMake {
    $ausPfad = Get-Command cmake -ErrorAction SilentlyContinue
    if ($ausPfad) { return $ausPfad.Source }
    $kandidaten = @()
    foreach ($stamm in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
        if (-not $stamm) { continue }
        foreach ($ausgabe in @('BuildTools', 'Community', 'Professional', 'Enterprise')) {
            $kandidaten += (Join-Path $stamm "Microsoft Visual Studio\2022\$ausgabe\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
        }
        $kandidaten += (Join-Path $stamm 'CMake\bin\cmake.exe')
    }
    foreach ($k in $kandidaten) { if (Test-Path -LiteralPath $k) { return $k } }
    return $null
}

function Bau-Abbruch {
    <# Ein gescheiterter Bau darf seine Ursache nicht mitnehmen. Die rohe
       Ausgabe wandert neben das Manifest, die Fehlerzeilen zusaetzlich auf
       die Konsole - sonst steht da nur "Exit 1" und niemand weiss warum.
       (Selbst erlebt am 20.08.: der Runner verschluckte den Compilerfehler.) #>
    param([string] $Schritt, [pscustomobject] $Lauf)

    $roh = ($Lauf.StdOut + "`n" + $Lauf.StdErr).Trim()
    $logDatei = Join-Path $Wurzel ('tmp\bau-fehler-{0}.log' -f $Beginn.ToString('yyyy-MM-dd-HHmm'))
    $logOrdner = Split-Path -Parent $logDatei
    if (-not (Test-Path -LiteralPath $logOrdner)) { New-Item -ItemType Directory -Path $logOrdner -Force | Out-Null }
    Set-Content -LiteralPath $logDatei -Value $roh -Encoding utf8

    $fehlerzeilen = @($roh -split "`r?`n" | Where-Object { $_ -match 'error|fehler' } | Select-Object -First 12)
    Write-Host ''
    Write-Host ("Bau ({0}) fehlgeschlagen - Exit {1}. Die ersten Fehlerzeilen:" -f $Schritt, $Lauf.ExitCode) -ForegroundColor Red
    foreach ($zeile in $fehlerzeilen) { Write-Host ("  " + $zeile.Trim()) -ForegroundColor Red }
    Write-Host ("Vollstaendiges Bauprotokoll: {0}" -f (RelativZurWurzel $logDatei)) -ForegroundColor Yellow
    Write-Host ''

    throw ("Bau fehlgeschlagen (Exit {0}) - kein Beweislauf auf kaputtem Baustand. Protokoll: {1}" `
           -f $Lauf.ExitCode, (RelativZurWurzel $logDatei))
}

if ($Bauen) {
    $cmakeBefehl = Finde-CMake
    if (-not $cmakeBefehl) {
        throw 'cmake nicht gefunden - weder im PATH noch unter Visual Studio 2022. -Bauen ist ohne cmake nicht moeglich.'
    }

    $bauVerzeichnis = Join-Path $Wurzel 'eq-copilot\build'
    $loesung = Join-Path $bauVerzeichnis 'EqCopilotSuite.sln'
    if (-not (Test-Path -LiteralPath $loesung)) {
        Write-Host 'Konfiguriere (erstmalig) ...' -ForegroundColor DarkGray
        $k = Fuehre-Aus -Datei $cmakeBefehl -Argumente @('-S', 'eq-copilot', '-B', 'eq-copilot/build', '-G', 'Visual Studio 17 2022', '-A', 'x64')
        $bauProtokoll += [pscustomobject]@{ Schritt = 'configure'; ExitCode = $k.ExitCode; StdOut = $k.StdOut; StdErr = $k.StdErr; Sekunden = $k.Sekunden }
        if ($k.ExitCode -ne 0) { Bau-Abbruch -Schritt 'configure' -Lauf $k }
    }

    # Nur Ziele bauen, die es im CMakeLists wirklich gibt (geplante Tests noch nicht).
    $cmakeText = Get-Content -LiteralPath (Join-Path $Wurzel 'eq-copilot\plugin\CMakeLists.txt') -Raw
    # `Ist-Stillgelegt` steht VOR dem Textvergleich, nicht dahinter: ein
    # stillgelegtes Bein darf auch dann nicht gebaut werden, wenn sein Name im
    # CMakeLists noch in einem Kommentar steht (S9b/SONDE-007c). Sonst suchte
    # der Bau ein Ziel, das es nicht mehr gibt, und braeche ab.
    $zuBauen = @($kanon | Where-Object { $_.Art -eq 'plugin' -and -not (Ist-Stillgelegt $_) -and $cmakeText -match [regex]::Escape($_.Name) } | ForEach-Object { $_.Name })
    $zuBauen += @($gemesseneZiele | Where-Object { $cmakeText -match [regex]::Escape($_.Marker) } | ForEach-Object { $_.Ziel })
    Write-Host ('Baue: ' + ($zuBauen -join ', ')) -ForegroundColor DarkGray
    $b = Fuehre-Aus -Datei $cmakeBefehl -Argumente (@('--build', 'eq-copilot/build', '--config', 'Release', '--target') + $zuBauen)
    $bauProtokoll += [pscustomobject]@{ Schritt = 'build'; ExitCode = $b.ExitCode; StdOut = $b.StdOut; StdErr = $b.StdErr; Sekunden = $b.Sekunden }
    if ($b.ExitCode -ne 0) { Bau-Abbruch -Schritt 'build' -Lauf $b }

    # S14-15/SONDE-010: das Lastbein A22 faehrt die C++-Clients gegen den
    # ECHTEN Rust-Listener. Dessen Probe-Binary entsteht nicht beim `cargo
    # test` von A4 (das baut nur Debug-Testbinaries), also wird es hier
    # ausdruecklich mitgebaut. Ohne diese Zeile pruefte A22 gegen ein altes
    # Release-Artefakt oder gar nicht - beides waere ein stiller Verlust.
    $cargoRelease = Fuehre-Aus -Datei 'cargo' -Argumente @(
        'build', '--release', '--manifest-path', 'broker/Cargo.toml',
        '--bin', 'eqcop-broker-v3probe', '--color', 'never')
    $bauProtokoll += [pscustomobject]@{ Schritt = 'cargo-release'; ExitCode = $cargoRelease.ExitCode; StdOut = $cargoRelease.StdOut; StdErr = $cargoRelease.StdErr; Sekunden = $cargoRelease.Sekunden }
    if ($cargoRelease.ExitCode -ne 0) { Bau-Abbruch -Schritt 'cargo-release' -Lauf $cargoRelease }
}

# ------------------------------------------------------------------ Kopfdaten

$juceGepinnt = 'nicht ermittelbar'
$cmakeWurzelDatei = Join-Path $Wurzel 'eq-copilot\CMakeLists.txt'
if (Test-Path -LiteralPath $cmakeWurzelDatei) {
    $treffer = Select-String -LiteralPath $cmakeWurzelDatei -Pattern 'GIT_TAG\s+(\S+)' | Select-Object -First 1
    if ($treffer) { $juceGepinnt = $treffer.Matches[0].Groups[1].Value }
}

$juceQuelle = Join-Path $Wurzel 'eq-copilot\build\_deps\juce-src'
$jucePlatte = 'nicht gebaut'
if (Test-Path -LiteralPath $juceQuelle) {
    $r = Fuehre-Aus -Datei 'git' -Argumente @('--no-optional-locks', '-C', $juceQuelle, 'describe', '--tags', '--always', '--dirty')
    $jucePlatte = if ($r.ExitCode -eq 0 -and $r.StdOut.Trim()) { $r.StdOut.Trim() } else { 'vorhanden, Version nicht ermittelbar' }
}

$flVersionen = @()
$ilStamm = 'C:\Program Files\Image-Line'
if (Test-Path -LiteralPath $ilStamm) {
    foreach ($ordner in (Get-ChildItem -LiteralPath $ilStamm -Directory -Filter 'FL Studio *' -ErrorAction SilentlyContinue)) {
        $flExe = Join-Path $ordner.FullName 'FL64.exe'
        if (Test-Path -LiteralPath $flExe) {
            $flVersionen += ('{0} {1}' -f $ordner.Name, (Get-Item -LiteralPath $flExe).VersionInfo.ProductVersion)
        }
    }
}
if ($flVersionen.Count -eq 0) { $flVersionen = @('keine FL-Installation gefunden') }

$schmutzig = Git-Wert @('status', '--porcelain')
$schmutzigeDateien = @()
if ($schmutzig -and $schmutzig -ne 'nicht ermittelbar') {
    $schmutzigeDateien = @($schmutzig -split "`r?`n" | Where-Object { $_.Trim() })
}

$kopf = [ordered]@{
    'Zeitpunkt'       = $Beginn.ToString('yyyy-MM-dd HH:mm:ss zzz')
    'Rechner'         = ('{0} | Windows {1}' -f $env:COMPUTERNAME, [Environment]::OSVersion.Version.ToString())
    'Zweig'           = (Git-Wert @('rev-parse', '--abbrev-ref', 'HEAD'))
    'Commit'          = (Git-Wert @('log', '-1', '--format=%h %s'))
    'Commit (voll)'   = (Git-Wert @('rev-parse', 'HEAD'))
    'Arbeitsbaum'     = $(if ($schmutzigeDateien.Count -eq 0) { 'sauber' } else { '{0} unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit' -f $schmutzigeDateien.Count })
    'JUCE gepinnt'    = $juceGepinnt
    'JUCE auf Platte' = $jucePlatte
    'FL Studio'       = ($flVersionen -join ' | ')
    'cargo'           = (Einzeilig 'cargo' @('--version'))
    'rustc'           = (Einzeilig 'rustc' @('--version'))
    'PowerShell'      = $PSVersionTable.PSVersion.ToString()
}
if ($Bauen -and $cmakeBefehl) { $kopf['cmake'] = (Einzeilig $cmakeBefehl @('--version')) }

# ------------------------------------------------------------------ Baustand

# Vollstaendig halten: JEDE Quelle, aus der eine Pruefbinaerdatei entsteht.
# Fehlt ein Ort, beglaubigt der Runner eine veraltete Messung als frisch -
# genau der T2-Befund vom 20.08., nur eine Ebene tiefer. hostbridge/, spike/,
# probe/, cmake/ und der JUCE-Bridge-Patch kamen mit SONDE-003/004a dazu,
# vertrag/ mit SONDE-005a.
#
# ⚠️ Nachgezogen am 23.08. (S10-11/SONDE-008, Selbstaudit): DREI Orte fehlten.
# `core/` ist neu in diesem Ticket - aber `state/` (seit SONDE-006, seit S8 der
# halbe NakamaKern) und `sonde/` (seit S9 die Quelle BEIDER neuen Bundles)
# fehlten seit ihrem jeweiligen Ticket. Der Riegel haette eine Aenderung an
# `NakamaState.cpp` oder `SondeProcessor.cpp` nicht bemerkt und einen veralteten
# Lauf als frisch beglaubigt - genau der Fehler, gegen den er errichtet wurde.
# Der Kommentar oben sagt "JEDE Quelle"; zwei Tickets lang stimmte das nicht.
$quellOrte = @(
    (Join-Path $Wurzel 'eq-copilot\plugin\src'),
    (Join-Path $Wurzel 'eq-copilot\plugin\core'),
    (Join-Path $Wurzel 'eq-copilot\plugin\state'),
    (Join-Path $Wurzel 'eq-copilot\plugin\sonde'),
    (Join-Path $Wurzel 'eq-copilot\plugin\tests'),
    (Join-Path $Wurzel 'eq-copilot\plugin\hostbridge'),
    (Join-Path $Wurzel 'eq-copilot\plugin\vertrag'),
    (Join-Path $Wurzel 'eq-copilot\plugin\hostprobe'),
    (Join-Path $Wurzel 'eq-copilot\plugin\spike'),
    (Join-Path $Wurzel 'eq-copilot\plugin\probe'),
    (Join-Path $Wurzel 'eq-copilot\cmake'),
    (Join-Path $Wurzel 'third_party\patches'),
    (Join-Path $Wurzel 'eq-copilot\plugin\CMakeLists.txt'),
    (Join-Path $Wurzel 'eq-copilot\CMakeLists.txt')
)
$neuesteQuelle = $null
foreach ($ort in $quellOrte) {
    if (-not (Test-Path -LiteralPath $ort)) { continue }
    $dateien = if ((Get-Item -LiteralPath $ort) -is [IO.DirectoryInfo]) {
        Get-ChildItem -LiteralPath $ort -Recurse -File -ErrorAction SilentlyContinue
    } else {
        @(Get-Item -LiteralPath $ort)
    }
    foreach ($d in $dateien) {
        if ($null -eq $neuesteQuelle -or $d.LastWriteTime -gt $neuesteQuelle) { $neuesteQuelle = $d.LastWriteTime }
    }
}

# Hat -Bauen soeben erfolgreich gebaut, ist die Frage entschieden: das
# Buildsystem hat die Abhaengigkeiten geprueft, und sein Urteil schlaegt den
# Zeitstempelvergleich. Ohne -Bauen bleibt die mtime die ehrliche Heuristik.
# (Sonst meldete jede fremde Datei im tests-Ordner die Binaries dauerhaft als
#  veraltet, obwohl ein No-op-Bau die Zeitstempel gar nicht anfasst.)
$bauBestaetigt = $Bauen -and (@($bauProtokoll | Where-Object { $_.Schritt -eq 'build' -and $_.ExitCode -eq 0 }).Count -gt 0)

$baustand = @()
$veraltet = $false
foreach ($eintrag in ($kanon | Where-Object { $_.Art -eq 'plugin' -and -not (Ist-Stillgelegt $_) })) {
    # Stillgelegte Beine bleiben hier draussen (S9b/SONDE-007c): ihre alte
    # .exe liegt nach dem letzten Bau noch im Baumverzeichnis und waere ab
    # sofort dauerhaft "aelter als die Quellen" - der Runner verweigerte dann
    # jede Beglaubigung wegen eines Binaerbildes, das gar nicht mehr gemessen
    # wird.
    $exe = Pruefbinaer $eintrag.Name
    if (-not (Test-Path -LiteralPath $exe)) { continue }
    $datei = Get-Item -LiteralPath $exe
    $istVeraltet = (-not $bauBestaetigt) `
                   -and ($null -ne $neuesteQuelle -and $datei.LastWriteTime -lt $neuesteQuelle)
    if ($istVeraltet) { $veraltet = $true }
    $baustand += [pscustomobject]@{
        Name   = $eintrag.Name
        Gebaut = $datei.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss')
        Hash   = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.Substring(0, 16)
        Stand  = $(if ($istVeraltet) { 'VERALTET' } elseif ($bauBestaetigt) { 'frisch (Bau bestaetigt)' } else { 'frisch' })
    }
}

# --------------------------------------------------------------- Kanon fahren

$ergebnisse = @()
$rot = 0
$fehlendeVoraussetzung = 0

foreach ($eintrag in $kanon) {

    $zeile = [pscustomobject]@{
        Kuerzel    = $eintrag.Kuerzel
        Name       = $eintrag.Name
        Behauptung = $eintrag.Behauptung
        Befehl     = ''
        Status     = ''
        Symbol     = ''
        ExitCode   = $null
        Sekunden   = $null
        StdOut     = ''
        StdErr     = ''
        Gelaufen   = $false
    }

    # Stillgelegt: nicht gefahren, nicht gezaehlt - aber sichtbar. Die Zeile
    # steht mit Datum und Grund in der Uebersichtstabelle jedes Manifests
    # (S9b/SONDE-007c). Weder gruen noch rot noch "geplant": ein stillgelegtes
    # Bein kommt nicht wieder, ein geplantes schon.
    if (Ist-Stillgelegt $eintrag) {
        $zeile.Symbol = '[STILLGELEGT]'
        $zeile.Status = $eintrag.Stillgelegt
        $zeile.Befehl = '(nicht gefahren)'
        $ergebnisse += $zeile
        Write-Host ('[STILLGELEGT] {0} - {1}' -f $zeile.Name, $eintrag.Stillgelegt) -ForegroundColor DarkGray
        continue
    }

    if ($eintrag.Art -eq 'cargo') {
        $zeile.Befehl = 'cargo ' + ($eintrag.Argumente -join ' ')
        if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
            $zeile.Symbol = '[FEHLT]'
            $zeile.Status = 'cargo nicht gefunden'
            $fehlendeVoraussetzung++
            $ergebnisse += $zeile
            Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            continue
        }
        $lauf = Fuehre-Aus -Datei 'cargo' -Argumente $eintrag.Argumente
    }
    elseif ($eintrag.Art -eq 'python') {
        $skript = Join-Path $Wurzel ('tools\eq-copilot\' + $eintrag.Name)
        $argumente = @('-3.13', $skript) + $eintrag.Argumente
        $zeile.Befehl = 'py -3.13 ' + (RelativZurWurzel $skript) + $(if ($eintrag.Argumente.Count -gt 0) { ' ' + ($eintrag.Argumente -join ' ') } else { '' })

        if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
            $zeile.Symbol = '[FEHLT]'
            $zeile.Status = 'py-Launcher nicht gefunden'
            $fehlendeVoraussetzung++
            $ergebnisse += $zeile
            Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            continue
        }
        if (-not (Test-Path -LiteralPath $skript)) {
            $zeile.Symbol = '[FEHLT]'
            $zeile.Status = 'Skript nicht gefunden'
            $fehlendeVoraussetzung++
            $ergebnisse += $zeile
            Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            continue
        }

        $lauf = Fuehre-Aus -Datei 'py' -Argumente $argumente

        # Exit 3 heisst in diesen Skripten "Voraussetzung fehlt" (z. B. das
        # Paket `jsonschema`), nicht "Behauptung widerlegt". Das ist ein
        # Unterschied, den ein Beweismanifest nicht verwischen darf.
        if ($lauf.ExitCode -eq 3) {
            $zeile.Befehl   = $zeile.Befehl
            $zeile.ExitCode = 3
            $zeile.Sekunden = $lauf.Sekunden
            $zeile.StdOut   = $lauf.StdOut
            $zeile.StdErr   = $lauf.StdErr
            $zeile.Gelaufen = $true
            $zeile.Symbol   = '[FEHLT]'
            $zeile.Status   = 'Voraussetzung fehlt (Exit 3)'
            $fehlendeVoraussetzung++
            $ergebnisse += $zeile
            Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            continue
        }
    }
    else {
        $exe = Pruefbinaer $eintrag.Name
        $zeile.Befehl = (RelativZurWurzel $exe) + $(if ($eintrag.Argumente.Count -gt 0) { ' ' + (($eintrag.Argumente | ForEach-Object { RelativZurWurzel $_ }) -join ' ') } else { '' })

        if (-not (Test-Path -LiteralPath $exe)) {
            if ($eintrag.AbPhase -eq 'jetzt') {
                $zeile.Symbol = '[FEHLT]'
                $zeile.Status = 'nicht gebaut - mit -Bauen nachziehen'
                $fehlendeVoraussetzung++
                Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
            }
            else {
                $zeile.Symbol = '[GEPLANT]'
                $zeile.Status = ('geplant (ab {0})' -f $eintrag.AbPhase)
            }
            $ergebnisse += $zeile
            continue
        }

        # Golden braucht die nicht eingecheckten Fixture-WAVs.
        if ($eintrag.Name -eq 'EqCopGoldenTest') {
            $wavs = @(Get-ChildItem -LiteralPath $fixtureVerzeichnis -Filter '*.wav' -ErrorAction SilentlyContinue)
            if ($wavs.Count -eq 0) {
                $zeile.Symbol = '[FEHLT]'
                $zeile.Status = 'Fixture-WAVs fehlen - py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav'
                $fehlendeVoraussetzung++
                $ergebnisse += $zeile
                Write-Host ('[FEHLT] {0} - {1}' -f $zeile.Name, $zeile.Status) -ForegroundColor Yellow
                continue
            }
        }

        $lauf = Fuehre-Aus -Datei $exe -Argumente $eintrag.Argumente
    }

    $zeile.Gelaufen = $true
    $zeile.ExitCode = $lauf.ExitCode
    $zeile.Sekunden = $lauf.Sekunden
    $zeile.StdOut = $lauf.StdOut
    $zeile.StdErr = $lauf.StdErr
    if ($lauf.ExitCode -eq 0) {
        $zeile.Symbol = '[OK]'
        $zeile.Status = 'Exit 0'
        Write-Host ('[OK] {0} - Exit 0 ({1})' -f $zeile.Name, (Dauertext $zeile.Sekunden)) -ForegroundColor Green
    }
    else {
        $zeile.Symbol = '[ROT]'
        $zeile.Status = ('Exit {0}' -f $lauf.ExitCode)
        $rot++
        Write-Host ('[ROT] {0} - Exit {1} ({2})' -f $zeile.Name, $lauf.ExitCode, (Dauertext $zeile.Sekunden)) -ForegroundColor Red
    }

    $ergebnisse += $zeile
}

# -------------------------------------------------------------------- Urteil

$gelaufen = @($ergebnisse | Where-Object { $_.Gelaufen })
$gruen = @($gelaufen | Where-Object { $_.ExitCode -eq 0 })
$geplant = @($ergebnisse | Where-Object { $_.Symbol -eq '[GEPLANT]' })
$stillgelegt = @($ergebnisse | Where-Object { $_.Symbol -eq '[STILLGELEGT]' })
# Ein "4/4 gruen" waere geschoenigt, solange sieben Kanon-Eintraege nur geplant sind.
$nachsatz = if ($geplant.Count -gt 0) { " | $($geplant.Count) geplante Pruefung(en) noch nicht gebaut" } else { '' }
# Und eine gesunkene Zahl ohne Erklaerung waere die andere Schoenung: das
# Urteil sagt selbst, wie viele Beine stillgelegt sind (S9b/SONDE-007c).
if ($stillgelegt.Count -gt 0) { $nachsatz += " | $($stillgelegt.Count) stillgelegte(s) Bein(e), siehe Uebersicht" }

$exitcode = 0
if ($rot -gt 0) {
    $exitcode = 2
    $urteil = "ROT - $rot von $($gelaufen.Count) Kanon-Laeufen fehlgeschlagen$nachsatz"
}
elseif ($fehlendeVoraussetzung -gt 0) {
    $exitcode = 3
    $urteil = "UNVOLLSTAENDIG - $($gruen.Count) gruen, $fehlendeVoraussetzung Voraussetzung(en) fehlen$nachsatz"
}
elseif ($veraltet) {
    $exitcode = 4
    $urteil = "NICHT BEGLAUBIGT - $($gruen.Count)/$($gelaufen.Count) gruen, aber Pruefbinaries sind aelter als die Quellen$nachsatz"
}
else {
    $urteil = "GRUEN - $($gruen.Count)/$($gelaufen.Count) Kanon-Laeufe bestanden$nachsatz"
}

# ------------------------------------------------------------------ Manifest

if (-not $Ziel) {
    $Ziel = Join-Path $Wurzel ('docs\beweise\lauf-{0}.md' -f $Beginn.ToString('yyyy-MM-dd-HHmm'))
}
elseif (-not [IO.Path]::IsPathRooted($Ziel)) {
    $Ziel = Join-Path $Wurzel $Ziel
}
$zielVerzeichnis = Split-Path -Parent $Ziel
if (-not (Test-Path -LiteralPath $zielVerzeichnis)) { New-Item -ItemType Directory -Path $zielVerzeichnis -Force | Out-Null }

# --- Rohausgabe-Datei bestimmen (NAK-96) ------------------------------------
# Vor dem Umbau hing jeder Lauf ~3000 Zeilen Rohtext an das Manifest; nach
# wenigen Laeufen war das Manifest fuer Prueferin und Dirigent nicht mehr
# lesbar. Der Lesetext (Kopf, Urteil, Uebersicht) bleibt im Manifest, die
# rohen stdout/stderr wandern in eine eigene Datei je Lauf.
#
# Der Name traegt den Stand, den der Lauf beweist: TICKET-<sha7>, bei
# unbestaetigtem Arbeitsbaum zusaetzlich `-dirty` - ein Lauf auf schmutzigem
# Baum beweist eben NICHT allein den Commit. Existiert die Datei schon
# (zweiter Lauf auf demselben Stand), zaehlt ein Suffix hoch. Es wird nie
# eine bestehende Rohausgabe ueberschrieben: ein Beweis, den ein spaeterer
# Lauf still ersetzt, ist kein Beweis.
$rohVerzeichnis = Join-Path $Wurzel 'docs\beweise\roh'
if (-not (Test-Path -LiteralPath $rohVerzeichnis)) { New-Item -ItemType Directory -Path $rohVerzeichnis -Force | Out-Null }

$rohStand = $kopf['Commit (voll)']
if ($rohStand -and $rohStand -ne 'nicht ermittelbar' -and $rohStand.Length -ge 7) {
    $rohStand = $rohStand.Substring(0, 7)
}
else {
    $rohStand = 'ohne-sha'
}
if ($schmutzigeDateien.Count -gt 0) { $rohStand = "$rohStand-dirty" }

$rohBasis = '{0}-{1}' -f ([IO.Path]::GetFileNameWithoutExtension($Ziel)), $rohStand
$rohDatei = Join-Path $rohVerzeichnis ($rohBasis + '.md')
$rohZaehler = 2
while (Test-Path -LiteralPath $rohDatei) {
    $rohDatei = Join-Path $rohVerzeichnis ('{0}-{1}.md' -f $rohBasis, $rohZaehler)
    $rohZaehler++
}

# Relativ gerechnet statt zusammengeklebt: ein Manifest darf auch in einem
# Unterordner von docs/beweise/ liegen, dann zeigt der Verweis nach oben.
$rohVerweis      = ([IO.Path]::GetRelativePath($zielVerzeichnis, $rohDatei)) -replace '\\', '/'
$manifestVerweis = ([IO.Path]::GetRelativePath($rohVerzeichnis, $Ziel)) -replace '\\', '/'

# Wortgleich in beiden Dateien. `tools/plan/planstand.py` (KANON, Zeile 66)
# liest die Kanon-Zahl aus GENAU diesem Wortlaut zurueck - der Verweis wird
# deshalb nur angehaengt, nie in die Zeile hineingeschrieben.
$laufZeile = "**Lauf:** $($Beginn.ToString('yyyy-MM-dd HH:mm')) | **Runner:** ``tools/beweise.ps1`` | **Urteil:** $urteil | **Exitcode:** $exitcode"

# --- Manifest: Lesetext ------------------------------------------------------

$z = [Collections.Generic.List[string]]::new()

if ($Anhaengen -and (Test-Path -LiteralPath $Ziel)) {
    $z.Add('')
    $z.Add('---')
    $z.Add('')
    $z.Add("## Kanon-Lauf - $Titel")
}
else {
    $z.Add("# Beweismanifest - $Titel")
}
$z.Add('')
$z.Add("$laufZeile | **Rohausgabe:** [$rohVerweis]($rohVerweis)")
$z.Add('')
$z.Add('### Kopf - woran gemessen wurde')
$z.Add('')
$z.Add('| Feld | Wert |')
$z.Add('|---|---|')
foreach ($schluessel in $kopf.Keys) { $z.Add("| $schluessel | $(Zellentext $kopf[$schluessel]) |") }
$z.Add('')

if ($veraltet) {
    $z.Add('> **VERALTET - dieser Lauf beweist NICHT den aktuellen Quellstand.**')
    $z.Add('> Mindestens eine Pruefbinaerdatei ist aelter als die Quellen. Neu fahren mit `-Bauen`.')
    $z.Add('')
}

$z.Add('### Uebersicht')
$z.Add('')
$z.Add('| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |')
$z.Add('|---|---|---|---|---|---|')
foreach ($e in $ergebnisse) {
    $dauer = if ($null -ne $e.Sekunden) { Dauertext $e.Sekunden } else { '-' }
    $link = if ($e.Gelaufen) { "[$($e.Kuerzel)]($rohVerweis#$($e.Kuerzel.ToLower()))" } else { '-' }
    $z.Add("| $($e.Kuerzel) | $(Zellentext $e.Behauptung) | ``$(Zellentext $e.Befehl)`` | $($e.Symbol) $(Zellentext $e.Status) | $dauer | $link |")
}
$z.Add('')

# --- Rohausgabe: alles, was der Lauf gesehen hat -----------------------------

$roh = [Collections.Generic.List[string]]::new()

$roh.Add("# Rohausgabe - $Titel")
$roh.Add('')
$roh.Add($laufZeile)
$roh.Add('')
$roh.Add("**Manifest:** [$manifestVerweis]($manifestVerweis)")
$roh.Add('')
$roh.Add('### Kopf - woran gemessen wurde')
$roh.Add('')
$roh.Add('| Feld | Wert |')
$roh.Add('|---|---|')
foreach ($schluessel in $kopf.Keys) { $roh.Add("| $schluessel | $(Zellentext $kopf[$schluessel]) |") }
$roh.Add('')

if ($schmutzigeDateien.Count -gt 0) {
    $roh.Add('<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>')
    $roh.Add('')
    $roh.Add((Block ($schmutzigeDateien -join "`n")))
    $roh.Add('')
    $roh.Add('</details>')
    $roh.Add('')
}

$roh.Add('### Baustand der Pruefbinaries')
$roh.Add('')
if ($baustand.Count -eq 0) {
    $roh.Add('_Keine Pruefbinaries vorhanden._')
}
else {
    $roh.Add('| Binaerdatei | gebaut am | SHA-256 (16) | Stand |')
    $roh.Add('|---|---|---|---|')
    foreach ($b in $baustand) { $roh.Add("| ``$($b.Name)`` | $($b.Gebaut) | ``$($b.Hash)`` | $($b.Stand) |") }
    $roh.Add('')
    # Die Liste kommt aus $quellOrte selbst, statt danebengeschrieben zu werden:
    # eine abgeschriebene Aufzaehlung altert genau dann, wenn ein Ort dazukommt -
    # also in dem Moment, in dem sie wichtig waere (Selbstaudit 23.08.).
    $orteText = (($quellOrte | ForEach-Object { '`' + (RelativZurWurzel $_).Replace('\', '/').Replace('eq-copilot/', '') + '`' }) -join ', ')
    $roh.Add("Neueste Quelldatei ($orteText): **$(if ($neuesteQuelle) { $neuesteQuelle.ToString('yyyy-MM-dd HH:mm:ss') } else { 'nicht ermittelbar' })**. ``cargo test`` uebersetzt selbst und ist damit immer frisch.")
    if ($bauBestaetigt) {
        $roh.Add('')
        $roh.Add('Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.')
    }
}
$roh.Add('')
if ($veraltet) {
    $roh.Add('> **VERALTET - dieser Lauf beweist NICHT den aktuellen Quellstand.**')
    $roh.Add('> Mindestens eine Pruefbinaerdatei ist aelter als die Quellen. Neu fahren mit `-Bauen`.')
    $roh.Add('')
}

$roh.Add('### Uebersicht')
$roh.Add('')
$roh.Add('| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |')
$roh.Add('|---|---|---|---|---|---|')
foreach ($e in $ergebnisse) {
    $dauer = if ($null -ne $e.Sekunden) { Dauertext $e.Sekunden } else { '-' }
    $link = if ($e.Gelaufen) { "[↓ $($e.Kuerzel)](#$($e.Kuerzel.ToLower()))" } else { '-' }
    $roh.Add("| $($e.Kuerzel) | $(Zellentext $e.Behauptung) | ``$(Zellentext $e.Befehl)`` | $($e.Symbol) $(Zellentext $e.Status) | $dauer | $link |")
}
$roh.Add('')

$roh.Add('### Rohe Ausgaben')
$roh.Add('')
foreach ($e in ($ergebnisse | Where-Object { $_.Gelaufen })) {
    $roh.Add("<a id=`"$($e.Kuerzel.ToLower())`"></a>")
    $roh.Add("#### $($e.Kuerzel) | $($e.Name)")
    $roh.Add('')
    $roh.Add("**Befehl:** ``$(Zellentext $e.Befehl)`` | **Exitcode:** $($e.ExitCode) | **Dauer:** $(Dauertext $e.Sekunden)")
    $roh.Add('')
    $roh.Add('stdout:')
    $roh.Add('')
    $roh.Add((Block $e.StdOut))
    $roh.Add('')
    $roh.Add('stderr:')
    $roh.Add('')
    $roh.Add((Block $e.StdErr))
    $roh.Add('')
}

if ($bauProtokoll.Count -gt 0) {
    $roh.Add('### Bau vor dem Lauf (`-Bauen`)')
    $roh.Add('')
    foreach ($b in $bauProtokoll) {
        $roh.Add("**$($b.Schritt)** | Exit $($b.ExitCode) | $(Dauertext $b.Sekunden)")
        $roh.Add('')
        $roh.Add('<details><summary>Rohe Ausgabe</summary>')
        $roh.Add('')
        $roh.Add((Block ($b.StdOut + "`n" + $b.StdErr)))
        $roh.Add('')
        $roh.Add('</details>')
        $roh.Add('')
    }
}

# Rohausgabe zuerst: sonst verweist ein geschriebenes Manifest auf eine Datei,
# die es nicht gibt, falls das Schreiben danach scheitert.
Set-Content -LiteralPath $rohDatei -Value ($roh -join "`n") -Encoding utf8

$inhalt = ($z -join "`n")
if ($Anhaengen -and (Test-Path -LiteralPath $Ziel)) {
    Add-Content -LiteralPath $Ziel -Value $inhalt -Encoding utf8
}
else {
    Set-Content -LiteralPath $Ziel -Value $inhalt -Encoding utf8
}

Write-Host ''
Write-Host $urteil
Write-Host ('Manifest:   {0}' -f (RelativZurWurzel $Ziel))
Write-Host ('Rohausgabe: {0}' -f (RelativZurWurzel $rohDatei))
exit $exitcode
