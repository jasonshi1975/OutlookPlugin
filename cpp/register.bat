@echo off
echo Registering AI Assistant Add-in...
echo.

REM Import registry entries (HKCU - no admin required)
reg import "%~dp0register.reg"

if %errorlevel% neq 0 (
    echo Failed to import registry entries.
    pause
    exit /b 1
)

echo Registry entries imported successfully.
echo.
echo Verifying registration...

reg query "HKCU\CLSID\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}"
reg query "HKCU\Software\Microsoft\Office\Outlook\Addins\AIAssistant"

echo.
echo ========================================
echo Registration COMPLETE.
echo Restart Outlook to load the add-in.
echo ========================================
pause