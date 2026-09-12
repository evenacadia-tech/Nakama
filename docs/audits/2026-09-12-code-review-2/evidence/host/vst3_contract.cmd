@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /EHsc /std:c++20 /MD /O2 /I"C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_processors\format_types\VST3_SDK" /Fe"vst3_contract.exe" vst3_contract.cpp
if errorlevel 1 exit /b 1
vst3_contract.exe
