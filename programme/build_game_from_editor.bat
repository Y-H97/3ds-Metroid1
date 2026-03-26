@echo off
REM Fuehrt Export und Nintendo-3DS-Build in einem Skript aus.
REM Dadurch muss der Workflow nicht mehr ueber mehrere Skripte verteilt manuell gestartet werden.
setlocal EnableExtensions

set "PROGRAMS_ROOT=%~dp0"
if "%PROGRAMS_ROOT:~-1%"=="\" set "PROGRAMS_ROOT=%PROGRAMS_ROOT:~0,-1%"
set "PROJECT_ROOT=%PROGRAMS_ROOT%\.."

echo ========================================
echo Vollstaendiger 3DS-Build aus dem Editor
echo ========================================
echo.

echo [1/2] Exportiere Level aus dem Editor ...
call "%PROGRAMS_ROOT%\export_level_maps.bat" --no-pause
if errorlevel 1 (
    echo.
    echo FEHLER: Der Level-Export ist fehlgeschlagen.
    pause
    exit /b 1
)

echo.
echo [2/2] Baue Nintendo-3DS-Version ...
call "%PROGRAMS_ROOT%\build_3ds.bat"
if errorlevel 1 (
    echo.
    echo FEHLER: Der 3DS-Build ist fehlgeschlagen.
    exit /b 1
)

echo.
echo Gesamtworkflow erfolgreich abgeschlossen.
echo Ausgabe liegt in:
echo   %PROJECT_ROOT%\3ds-cpp\3ds-cpp.3dsx
echo   %PROJECT_ROOT%\3ds-cpp\3ds-cpp.smdh
exit /b 0