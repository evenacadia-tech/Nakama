@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /O2 /EHsc /std:c++20 /utf-8 models_extra.cpp /Fe:models_extra.exe /Fo:models_extra.obj
