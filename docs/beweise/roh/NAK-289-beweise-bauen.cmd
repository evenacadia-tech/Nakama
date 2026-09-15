@echo off
rem NAK-289 Etappe 1: baut und faehrt die zwei Beweisprogramme der Fundklassen e) und f).
rem Aufruf aus einem beliebigen Verzeichnis; Bauartefakte landen neben diesem Skript
rem und gehoeren nicht ins Repo (nur .cpp und .txt sind Beleg).
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
cl /nologo /O2 /std:c++20 /EHsc /utf-8 /W4 NAK-289-rundung-beweis.cpp || exit /b 11
cl /nologo /O2 /std:c++20 /EHsc /utf-8 /W4 NAK-289-atoi-strtol-beweis.cpp || exit /b 12
rem Mit .\ davor: NoDefaultCurrentDirectoryInExePath kann die Suche im aktuellen Verzeichnis abschalten.
.\NAK-289-rundung-beweis.exe
echo EXIT=%errorlevel%
.\NAK-289-atoi-strtol-beweis.exe
echo EXIT=%errorlevel%
exit /b 0
