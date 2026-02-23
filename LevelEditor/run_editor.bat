@echo off
REM Startet den LevelEditor mit lokaler LÖVE-Installation unter Windows.
echo Starte Level Editor...

REM Nutze den absoluten Pfad zur LÖVE Installation
set LOVE_PATH="C:\Program Files\LOVE\love.exe"

REM Prüfe ob LÖVE an diesem Ort existiert
if not exist %LOVE_PATH% (
    echo FEHLER: Konnte love.exe nicht finden unter:
    echo %LOVE_PATH%
    echo Bitte passe den Pfad in dieser Datei an.
    pause
    exit /b
)

REM Wechsel auf ein temporäres Laufwerk (löst das UNC/Netzwerk-Pfad Problem)
pushd "%~dp0"

REM Starte LÖVE im aktuellen Verzeichnis (.)
%LOVE_PATH% .

REM Laufwerk wieder freigeben
popd

if %errorlevel% neq 0 (
    echo.
    echo LÖVE wurde beendet (Fehlercode: %errorlevel%)
    pause
)
?