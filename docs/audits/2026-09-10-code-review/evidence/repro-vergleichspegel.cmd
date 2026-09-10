@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 2
cl /nologo /std:c++20 /O2 /EHsc /W4 /utf-8 /I"C:\na-audit-20260910-844b9c1\eq-copilot\plugin\core\analysis" "C:\na-audit-20260910-844b9c1\.workflow\ultracode\20260910-audit\evidence\repro-vergleichspegel.cpp" /Fe:"C:\na-audit-20260910-844b9c1\.workflow\ultracode\20260910-audit\evidence\repro-vergleichspegel.exe" /Fo:"C:\na-audit-20260910-844b9c1\.workflow\ultracode\20260910-audit\evidence\repro-vergleichspegel.obj"
if errorlevel 1 exit /b 2
"C:\na-audit-20260910-844b9c1\.workflow\ultracode\20260910-audit\evidence\repro-vergleichspegel.exe"
exit /b %errorlevel%
