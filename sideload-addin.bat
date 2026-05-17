@echo off
echo ========================================
echo   Outlook AI Assistant - Sideload
echo ========================================
echo.
echo This script will help you sideload the Web Add-in.
echo.
echo Steps:
echo   1. Outlook will open the "Get Add-ins" page
echo   2. Click "My Add-ins" tab
echo   3. Click "Upload my add-in" or "Add a custom add-in"
echo   4. Enter this URL:
echo.
echo      https://jasonshi1975.github.io/OutlookPlugin/manifest.xml
echo.
echo   5. Click OK/Add
echo.
echo Press any key to open Outlook Get Add-ins page...
pause > nul

start "" "https://outlook.office.com/mail/deeplink/compose?addins"

echo.
echo Done! Follow the steps above.
pause