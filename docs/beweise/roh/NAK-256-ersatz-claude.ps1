# NAK-256, Bein (d3): Ersatz fuer claude.exe ueber den Pruefhaken NAKAMA_DIRIGENT_CLAUDE
# in tools/dirigent/start-dirigent.ps1. Runde 1 fordert wie der Dirigent (Skill §5) per
# Markerdatei einen Neustart im selben Fenster an, Runde 2 endet ohne Marker. Der Zaehler
# liegt im Probenordner NAK256_PROBE_DIR, nie im Repo.
$zaehler = Join-Path $env:NAK256_PROBE_DIR 'runden.txt'
$runde = 1
if ([IO.File]::Exists($zaehler)) { $runde = [int][IO.File]::ReadAllText($zaehler) + 1 }
[IO.File]::WriteAllText($zaehler, [string]$runde)
[Console]::WriteLine("ERSATZ-CLAUDE Runde $runde")
Start-Sleep -Seconds 6
if ($runde -eq 1) {
    [IO.File]::WriteAllText((Join-Path ([IO.Path]::GetTempPath()) 'nakama-dirigent-neustart.marker'), 'nak256-probe')
}
exit 0
