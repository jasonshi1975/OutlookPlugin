@echo off
echo Uninstalling AIAssistant C++ Add-in...

REM Delete registry keys
reg delete "HKCU\Software\Classes\CLSID\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}" /f 2>nul
reg delete "HKCU\Software\Classes\AIAssistant" /f 2>nul
reg delete "HKCU\Software\Microsoft\Office\Outlook\Addins\AIAssistant" /f 2>nul
reg delete "HKCU\Software\Microsoft\Office\16.0\Outlook\Addins\AIAssistant" /f 2>nul

echo Registry keys removed.
echo.
echo IMPORTANT: Close Outlook completely before continuing.
echo Then run this script again to confirm uninstallation.
pause