@echo off
REM devkitARM Installation Batch-Skript
REM Führt die Installation in einer MSYS2-Umgebung durch

echo === devkitARM Installation ===
echo.

REM Lock entfernen
del /F /Q "C:\devkitPro\msys2\var\lib\pacman\db.lck" 2>nul

REM Installation starten (mit automatischer Auswahl "all")
echo. | C:\devkitPro\msys2\usr\bin\bash.exe -lc "yes '' | pacman -S --needed --noconfirm devkitARM libctru citro3d citro2d 3dstools 3dslink tex3ds picasso"

echo.
echo === Prüfe Installation ===
C:\devkitPro\msys2\usr\bin\bash.exe -lc "arm-none-eabi-gcc --version"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === Installation erfolgreich! ===
    echo.
    echo Naechste Schritte:
    echo   cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp
    echo   make
) else (
    echo.
    echo === Installation fehlgeschlagen ===
    echo Siehe Fehlerausgabe oben.
)

pause
