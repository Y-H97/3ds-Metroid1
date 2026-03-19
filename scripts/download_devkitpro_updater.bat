@echo off
REM Öffnet die devkitPro Updater Download-Seite
REM Dies ist die einfachste Methode zur Installation von devkitARM

echo ========================================
echo devkitPro Updater Download
echo ========================================
echo.
echo Dieser Updater umgeht alle MSYS2/pacman-Probleme.
echo.
echo Schritte:
echo 1. Lade devkitProUpdater.exe herunter
echo 2. Fuehre die .exe aus
echo 3. Waehle "Nintendo 3DS" in der Liste
echo 4. Klicke "Install"
echo.
echo Oeffne jetzt den Browser...
echo.

start https://github.com/devkitPro/installer/releases/latest

echo.
echo Nach erfolgreicher Installation:
echo   cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp
echo   make
echo.
pause
