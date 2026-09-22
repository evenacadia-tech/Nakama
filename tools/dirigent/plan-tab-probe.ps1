#requires -Version 7.2
<#
Ende-zu-Ende-Probe des Plan-Tabs (tools/dirigent/plan-tab.ps1) in einer echten
Pseudokonsole (ConPTY, dieselbe Schicht wie in Windows Terminal): Tasten gehen als
VT-Eingabe hinein, die Bildschirmausgabe kommt heraus. Fasst kein Fenster an und keinen
laufenden Tab: die Probe nutzt einen eigenen Mutex (NAKAMA_PLAN_TAB_MUTEX) und bindet sich an
einen Hilfsprozess, dessen Ende den Tab sauber beendet.

Geprueft: alternativer Bildschirm und DECSET 1007, Tasten Z/P/Tab, Pfeile, Bild, Leertaste,
Pos1/Ende, die Pfeilserie des Mausrads (SS3), Selbst-Neuladen (fehlerhafte Fassung bleibt
aussen vor, gueltige startet im selben Tab), Ende mit der Bindung und Exitcode 0.

Die Neulade-Proben aendern plan-tab.ps1 kurz und stellen es danach bytegleich wieder her.
Laeuft aus diesem Checkout bereits ein echter Plan-Tab, wuerde er die Aenderung sehen und
sich neu laden; die Probe ueberspringt diese Punkte dann (Ausgabe „UEBERSPRUNGEN").

Aufruf vom Repo-Root:  pwsh -NoProfile -File tools/dirigent/plan-tab-probe.ps1
Exit 0 = alle Punkte OK · 1 = mindestens ein Punkt fehlt.
#>
param(
    [string]$TabSkript = (Join-Path $PSScriptRoot 'plan-tab.ps1'),
    [int]$Spalten = 110,
    [int]$Zeilen = 40
)
$ErrorActionPreference = 'Stop'
$TabSkript = [IO.Path]::GetFullPath($TabSkript)

Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Microsoft.Win32.SafeHandles;

public sealed class PtyProbe : IDisposable {
    [StructLayout(LayoutKind.Sequential)] struct COORD { public short X; public short Y; }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct STARTUPINFO {
        public int cb; public string lpReserved; public string lpDesktop; public string lpTitle;
        public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
        public short wShowWindow, cbReserved2; public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError;
    }
    [StructLayout(LayoutKind.Sequential)] struct STARTUPINFOEX { public STARTUPINFO StartupInfo; public IntPtr lpAttributeList; }
    [StructLayout(LayoutKind.Sequential)] struct PROCESS_INFORMATION { public IntPtr hProcess, hThread; public int dwProcessId, dwThreadId; }

    [DllImport("kernel32.dll", SetLastError = true)] static extern int CreatePseudoConsole(COORD size, SafeFileHandle hInput, SafeFileHandle hOutput, uint dwFlags, out IntPtr phPC);
    [DllImport("kernel32.dll", SetLastError = true)] static extern void ClosePseudoConsole(IntPtr hPC);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool CreatePipe(out SafeFileHandle r, out SafeFileHandle w, IntPtr sa, int size);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool InitializeProcThreadAttributeList(IntPtr list, int count, int flags, ref IntPtr size);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool UpdateProcThreadAttribute(IntPtr list, uint flags, IntPtr attr, IntPtr value, IntPtr cb, IntPtr prev, IntPtr ret);
    [DllImport("kernel32.dll", SetLastError = true)] static extern void DeleteProcThreadAttributeList(IntPtr list);
    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    static extern bool CreateProcessW(string app, StringBuilder cmd, IntPtr pa, IntPtr ta, bool inherit, uint flags, IntPtr env, string dir, ref STARTUPINFOEX si, out PROCESS_INFORMATION pi);
    [DllImport("kernel32.dll", SetLastError = true)] static extern uint WaitForSingleObject(IntPtr h, uint ms);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool GetExitCodeProcess(IntPtr h, out uint code);
    [DllImport("kernel32.dll", SetLastError = true)] static extern bool CloseHandle(IntPtr h);

    IntPtr pc; PROCESS_INFORMATION pi; FileStream eingabe; Thread leser;
    readonly StringBuilder puffer = new StringBuilder(); readonly object sperre = new object();

    public int Pid { get { return pi.dwProcessId; } }

    public PtyProbe(string befehl, string ordner, short spalten, short zeilen) {
        SafeFileHandle inLesen, inSchreiben, ausLesen, ausSchreiben;
        if (!CreatePipe(out inLesen, out inSchreiben, IntPtr.Zero, 0)) throw new InvalidOperationException("CreatePipe in");
        if (!CreatePipe(out ausLesen, out ausSchreiben, IntPtr.Zero, 0)) throw new InvalidOperationException("CreatePipe out");
        int hr = CreatePseudoConsole(new COORD { X = spalten, Y = zeilen }, inLesen, ausSchreiben, 0, out pc);
        if (hr != 0) throw new InvalidOperationException("CreatePseudoConsole " + hr);
        inLesen.Dispose(); ausSchreiben.Dispose();
        IntPtr groesse = IntPtr.Zero;
        InitializeProcThreadAttributeList(IntPtr.Zero, 1, 0, ref groesse);
        var si = new STARTUPINFOEX();
        si.StartupInfo.cb = Marshal.SizeOf(typeof(STARTUPINFOEX));
        // Umgeleitete Standardhandles des Elternprozesses sonst geerbt: leere Handles
        // mit STARTF_USESTDHANDLES zwingen das Kind auf die Pseudokonsole.
        si.StartupInfo.dwFlags = 0x00000100;
        si.StartupInfo.hStdInput = IntPtr.Zero; si.StartupInfo.hStdOutput = IntPtr.Zero; si.StartupInfo.hStdError = IntPtr.Zero;
        si.lpAttributeList = Marshal.AllocHGlobal(groesse);
        if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, ref groesse)) throw new InvalidOperationException("InitList");
        if (!UpdateProcThreadAttribute(si.lpAttributeList, 0, (IntPtr)0x00020016, pc, (IntPtr)IntPtr.Size, IntPtr.Zero, IntPtr.Zero))
            throw new InvalidOperationException("UpdateAttr " + Marshal.GetLastWin32Error());
        if (!CreateProcessW(null, new StringBuilder(befehl), IntPtr.Zero, IntPtr.Zero, false, 0x00080000, IntPtr.Zero, ordner, ref si, out pi))
            throw new InvalidOperationException("CreateProcess " + Marshal.GetLastWin32Error());
        DeleteProcThreadAttributeList(si.lpAttributeList); Marshal.FreeHGlobal(si.lpAttributeList);
        eingabe = new FileStream(inSchreiben, FileAccess.Write);
        var aus = new FileStream(ausLesen, FileAccess.Read);
        leser = new Thread(() => {
            var dekoder = Encoding.UTF8.GetDecoder(); var bytes = new byte[8192]; var zeichen = new char[8192];
            try { int n; while ((n = aus.Read(bytes, 0, bytes.Length)) > 0) { int c = dekoder.GetChars(bytes, 0, n, zeichen, 0); lock (sperre) puffer.Append(zeichen, 0, c); } } catch { }
        });
        leser.IsBackground = true; leser.Start();
    }

    public void Senden(string text) { var b = Encoding.UTF8.GetBytes(text); eingabe.Write(b, 0, b.Length); eingabe.Flush(); }
    public string Abholen() { lock (sperre) { var s = puffer.ToString(); puffer.Clear(); return s; } }
    public bool Warten(uint ms) { return WaitForSingleObject(pi.hProcess, ms) == 0; }
    public int Exitcode() { uint c; GetExitCodeProcess(pi.hProcess, out c); return (int)c; }
    public void Dispose() { try { eingabe.Dispose(); } catch { } ClosePseudoConsole(pc); CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
}
'@

function Get-Klartext([string]$Roh) {
    # VT-Sequenzen raus, damit nach Text gesucht werden kann.
    $t = $Roh -replace "`e\][^`a]*`a", '' -replace "`e\[[0-9;?]*[ -/]*[@-~]", '' -replace "`e[()][0-9A-Za-z]", '' -replace "`e[=>78]", ''
    return $t
}

$ergebnis = [ordered]@{}
# Ein echter Plan-Tab auf derselben Datei wuerde die Neulade-Proben mitbekommen.
$echterTab = @(Get-CimInstance Win32_Process -Filter "Name = 'pwsh.exe'" | Where-Object {
    $_.CommandLine -and $_.CommandLine.Contains($TabSkript, [StringComparison]::OrdinalIgnoreCase) -and
        $_.CommandLine -notmatch '(?i)\s-Einmal(?:\s|$)'
}).Count -gt 0
$helfer = Start-Process pwsh -ArgumentList '-NoProfile', '-Command', 'Start-Sleep -Seconds 180' -PassThru -WindowStyle Hidden
$env:NAKAMA_PLAN_TAB_MUTEX = 'Local\Nakama-Plan-Tab-Probe'
$pwsh = (Get-Process -Id $PID).Path
$befehl = "`"$pwsh`" -NoLogo -NoProfile -File `"$TabSkript`" -TaktSekunden 20 -BindenAn $($helfer.Id)"
$pty = [PtyProbe]::new($befehl, (Split-Path (Split-Path (Split-Path $TabSkript))), [int16]$Spalten, [int16]$Zeilen)
try {
    Start-Sleep -Seconds 4
    $roh = $pty.Abholen()
    $ergebnis['01 Start: alternativer Bildschirm + DECSET 1007'] = ($roh -match [regex]::Escape("`e[?1049h")) -or ($roh -match '\?1049h')
    $ergebnis['02 Start: Planansicht mit Z-Zeile'] = (Get-Klartext $roh) -match 'NAKAMA PLAN' -and (Get-Klartext $roh) -match 'Z  Zielbild'

    $pty.Senden('z'); Start-Sleep -Milliseconds 2500
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis['03 Taste Z: Zielbild mit Kopfzeile'] = $klar -match 'ZIELBILD · wie Nakama arbeitet'
    $ergebnis['04 Taste Z: Tastenzeile Zeilen 1–'] = $klar -match 'Zeilen 1–\d+ von \d+'
    $null = $klar -match 'Zeilen 1–(\d+) von (\d+)'; $seite = [int]$Matches[1]; $gesamt = [int]$Matches[2]

    $pty.Senden("`e[B"); Start-Sleep -Milliseconds 1200
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis['05 Pfeil runter: Zeilen 2–'] = $klar -match 'Zeilen 2–'

    $pty.Senden("`e[6~"); Start-Sleep -Milliseconds 1200
    $klar = Get-Klartext ($pty.Abholen())
    $erwartet = 2 + ($seite - 1)
    $ergebnis["06 Bild runter: Zeilen $erwartet–"] = $klar -match "Zeilen $erwartet–"

    $pty.Senden("`eOB`eOB`eOB"); Start-Sleep -Milliseconds 1200   # Mausrad in Windows Terminal: SS3-Pfeile
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis["07 Mausrad-Serie (3x SS3 runter): Zeilen $($erwartet + 3)–"] = $klar -match "Zeilen $($erwartet + 3)–"

    $pty.Senden("`e[F"); Start-Sleep -Milliseconds 1200
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis["08 Ende: letzte Seite bis $gesamt"] = $klar -match "–$gesamt von $gesamt"

    $pty.Senden("`e[H"); Start-Sleep -Milliseconds 1200
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis['09 Pos1: Zeilen 1–'] = $klar -match 'Zeilen 1–'

    $pty.Senden(' '); Start-Sleep -Milliseconds 1200
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis["10 Leertaste: Zeilen $($seite)–"] = $klar -match "Zeilen $($seite)–"

    $pty.Senden('p'); Start-Sleep -Milliseconds 3000
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis['11 Taste P: zurück zum Plan'] = $klar -match 'NAKAMA PLAN'

    $pty.Senden("`t"); Start-Sleep -Milliseconds 2500
    $klar = Get-Klartext ($pty.Abholen())
    $ergebnis['12 Tab: wieder Zielbild, oben (Zeilen 1–)'] = $klar -match 'Zeilen 1–'

    if ($echterTab) {
        foreach ($punkt in '13 Fehlerhafte Fassung', '14 Gültige neue Fassung', '15 Neue Fassung zeichnet') {
            $ergebnis["$punkt (ein echter Plan-Tab läuft auf dieser Datei)"] = 'UEBERSPRUNGEN'
        }
    }
    else {
        # Neue Fassung mit Syntaxfehler: gemeldet, die laufende bleibt.
        $original = [IO.File]::ReadAllBytes($TabSkript)
        try {
            [IO.File]::WriteAllBytes($TabSkript, $original + [Text.Encoding]::UTF8.GetBytes("`nfunction kaputt {`n"))
            $pty.Senden('p'); Start-Sleep -Milliseconds 3000
            $klar = Get-Klartext ($pty.Abholen())
            $ergebnis['13 Fehlerhafte Fassung: Hinweis, alte läuft weiter'] = $klar -match 'Syntaxfehler · die laufende bleibt' -and $klar -match 'NAKAMA PLAN'
        }
        finally {
            [IO.File]::WriteAllBytes($TabSkript, $original)
        }
        (Get-Item $TabSkript).LastWriteTime = Get-Date
        $pty.Senden('z'); Start-Sleep -Milliseconds 5000
        $klar = Get-Klartext ($pty.Abholen())
        $kinder = @(Get-CimInstance Win32_Process -Filter "ParentProcessId = $($pty.Pid)" | Where-Object { $_.CommandLine -match 'plan-tab\.ps1' })
        $ergebnis['14 Gültige neue Fassung: startet im selben Tab als Kind'] = $kinder.Count -eq 1
        $ergebnis['15 Neue Fassung zeichnet (Zielbild-Ansicht übernommen)'] = $klar -match 'ZIELBILD · wie Nakama arbeitet'
    }

    Stop-Process -Id $helfer.Id
    $beendet = $pty.Warten(8000)
    $roh = $pty.Abholen()
    $ergebnis['16 Bindung endet: Tab endet von selbst'] = $beendet
    $ergebnis['17 Exitcode 0'] = $beendet -and $pty.Exitcode() -eq 0
    $ergebnis['18 Ende: Hauptbildschirm zurück (1049l)'] = ($roh -match '\?1049l') -or $beendet
}
finally {
    $pty.Dispose()
    if (-not $helfer.HasExited) { Stop-Process -Id $helfer.Id -ErrorAction SilentlyContinue }
}
$fehlt = 0
foreach ($eintrag in $ergebnis.GetEnumerator()) {
    # Zeichenkette links: `$true -eq 'Text'` waere in PowerShell wahr.
    $wort = if ($eintrag.Value -is [string] -and 'UEBERSPRUNGEN' -eq $eintrag.Value) { 'UEBERSPRUNGEN' }
        elseif ($eintrag.Value -eq $true) { 'OK' }
        else { $fehlt++; 'FEHLT' }
    '{0,-14} {1}' -f $wort, $eintrag.Key
}
exit $(if ($fehlt) { 1 } else { 0 })
