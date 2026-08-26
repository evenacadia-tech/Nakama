[CmdletBinding()]
param(
    [switch]$Plain,
    [switch]$Caption
)

$escape = [char]27
$supportsColor = -not $Plain -and [bool]$Host.UI.SupportsVirtualTerminal

if ($supportsColor) {
    $shadow = "$escape[38;2;128;128;144m"
    $champagne = "$escape[38;2;224;208;192m"
    $gold = "$escape[38;2;208;176;144m"
    $glow = "$escape[38;2;191;255;241m"
    $reset = "$escape[0m"
}
else {
    $shadow = ''
    $champagne = ''
    $gold = ''
    $glow = ''
    $reset = ''
}

$logo = @'
{S}  ▄█▄                     ▄█▄{R}
{S}▄██ ▀█▄       {A}▄█▄{S}       ▄█▀ ██▄{R}
{S}██    ▀██▄▄▄{C}██▀ ▀██{S}▄▄▄██▀    ██{R}
{S}▀██▄       {C}▄█████████▄{S}       ▄██▀{R}
{S}  ▀██▄▄   {C}██  {G}◉   ◉{C}  ██{S}   ▄▄██▀{R}
{S}      ▀██▄{C}██         ██{S}▄██▀{R}
{C}         ▀████▄▄▄▄▄████▀{R}
{C}             ▀█████▀{R}
'@

$tokens = @{
    '{S}' = $shadow
    '{C}' = $champagne
    '{A}' = $gold
    '{G}' = $glow
    '{R}' = $reset
}

foreach ($line in ($logo -split "`r?`n")) {
    foreach ($token in $tokens.Keys) {
        $line = $line.Replace($token, $tokens[$token])
    }
    [Console]::WriteLine($line)
}

if ($Caption) {
    [Console]::WriteLine('')
    [Console]::WriteLine("$champagne                 N A K A M A$reset")
    [Console]::WriteLine("$gold          C L A U D E   D I R I G E N T$reset")
}
