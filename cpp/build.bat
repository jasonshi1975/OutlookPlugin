@echo off
REM Clean old files
del addin.obj 2>nul
del AIAssistant.dll 2>nul
del AIAssistant.lib 2>nul
del AIAssistant.exp 2>nul

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

cd /d "d:\Work\OutlookPlugin\cpp"

echo Compiling addin.cpp (64-bit for Outlook 64-bit)...
cl /c /EHsc /W3 /O2 /MD src\addin.cpp /Fo:addin.obj

echo Linking DLL (64-bit)...
link /DLL /OUT:"AIAssistant.dll" /DEF:"AIAssistant.def" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64" winhttp.lib ole32.lib oleaut32.lib uuid.lib user32.lib kernel32.lib advapi32.lib shell32.lib addin.obj

if exist AIAssistant.dll (
    echo Build SUCCESS!
    dir AIAssistant.dll
) else (
    echo Build FAILED!
)