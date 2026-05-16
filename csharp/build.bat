@echo off
echo Building AIAssistant C# Outlook Add-in...

cd /d "d:\Work\OutlookPlugin\csharp"

REM Build
dotnet build -c Release

if exist "bin\Release\net8.0-windows\AIAssistant.comhost.dll" (
    echo Build SUCCESS!
    echo.
    echo COM Host: bin\Release\net8.0-windows\AIAssistant.comhost.dll
    echo Main DLL: bin\Release\net8.0-windows\AIAssistant.dll
    echo.
    echo To register (run as Administrator):
    echo   1. Run: regedit /s register.reg
    echo   Or manually run: regsvr32 bin\Release\net8.0-windows\AIAssistant.comhost.dll
    echo.
    echo To unregister:
    echo   Run: regedit /s unregister.reg
    echo.
    echo IMPORTANT: Close Outlook before registering/unregistering!
) else (
    echo Build FAILED!
)

pause