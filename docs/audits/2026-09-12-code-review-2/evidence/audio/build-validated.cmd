@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d C:\na-audit-audio-20260912
cl /nologo /std:c++20 /O2 /EHsc /MD /D NDEBUG /D _NDEBUG /D JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 /D JUCE_STANDALONE_APPLICATION=1 /D JUCE_WEB_BROWSER=0 /D JUCE_USE_CURL=0 /D JUCE_MODULE_AVAILABLE_juce_core=1 /D JUCE_MODULE_AVAILABLE_juce_events=1 /D JUCE_MODULE_AVAILABLE_juce_data_structures=1 /D JUCE_MODULE_AVAILABLE_juce_cryptography=1 /I C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\dsp /I C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\src /I C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\core /I C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\state /I C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules repro-validated.cpp NakamaKern.lib flatbuffers.lib juce_core.obj juce_core_CompilationTime.obj juce_events.obj juce_data_structures.obj juce_cryptography.obj Advapi32.lib Bcrypt.lib Crypt32.lib Wintrust.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib /Fe:repro-validated.exe
if errorlevel 1 exit /b %errorlevel%
repro-validated.exe
