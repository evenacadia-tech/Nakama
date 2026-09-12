@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cl @"C:\Users\phili\AppData\Local\Temp\nakama-host-audit-20260912-w2\compile.rsp"
if errorlevel 1 exit /b 1
link @"C:\Users\phili\AppData\Local\Temp\nakama-host-audit-20260912-w2\link.rsp"
