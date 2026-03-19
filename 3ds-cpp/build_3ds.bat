@echo off
REM 3DS Build Script mit korrekten Umgebungsvariablen
REM Setzt devkitPro-Pfade und führt make durch

setlocal enabledelayedexpansion

echo ========================================
echo 3DS Build (Metroidvania)
echo ========================================
echo.

REM Setze Umgebungsvariablen
set DEVKITPRO=C:\devkitPro
set DEVKITARM=C:\devkitPro\devkitARM
set PATH=%DEVKITARM%\bin;%PATH%

echo [1/4] Umgebungsvariablen prüfen...
echo   DEVKITPRO: %DEVKITPRO%
echo   DEVKITARM: %DEVKITARM%

REM Compiler testen
echo [2/4] Compiler testen...
call %DEVKITARM%\bin\arm-none-eabi-gcc.exe --version | head -n 1
if errorlevel 1 (
    echo FEHLER: Compiler nicht gefunden!
    pause
    exit /b 1
)

REM Level-Export (optional)
echo [3/4] Maps werden verwendet aus romfs/...

REM Build durchführen
echo [4/4] Starte Build...
echo.
cd /d "%~dp0"
call make clean
call make

if errorlevel 1 (
    echo.
    echo *** BUILD FEHLGESCHLAGEN ***
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD ERFOLGREICH!
echo ========================================
echo.
echo Ausgabedateien:
echo   3ds-cpp.3dsx (Game-Datei für 3DS)
echo   3ds-cpp.smdh (Icon + Metadaten)
echo.
echo Nächste Schritte:
echo   1. SD-Karte in den PC einlegen
echo   2. Ordner erstellen: SD:\3ds\metroid\
echo   3. Diese Dateien dort kopieren:
echo      - 3ds-cpp.3dsx
echo      - 3ds-cpp.smdh
echo   4. SD-Karte zurück in 3DS
echo   5. Homebrew Launcher starten
echo.
pause
