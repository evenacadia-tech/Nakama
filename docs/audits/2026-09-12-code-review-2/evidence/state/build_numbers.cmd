@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" > "C:\Users\phili\AppData\Local\Temp\nakama-audit-state-publication-20260912\vcvars.log"
if errorlevel 1 exit /b 2
cl @"C:\Users\phili\AppData\Local\Temp\nakama-audit-state-publication-20260912\compile_numbers.rsp" > "C:\Users\phili\AppData\Local\Temp\nakama-audit-state-publication-20260912\compile_numbers.log" 2>&1
if errorlevel 1 exit /b 3
link @"C:\Users\phili\AppData\Local\Temp\nakama-audit-state-publication-20260912\link_numbers.rsp" > "C:\Users\phili\AppData\Local\Temp\nakama-audit-state-publication-20260912\link_numbers.log" 2>&1
if errorlevel 1 exit /b 4
