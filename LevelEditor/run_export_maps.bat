@echo off
setlocal EnableExtensions

set "NO_PAUSE="
if /I "%~1"=="--no-pause" set "NO_PAUSE=1"

echo Exportiere LevelEditor Maps nach 3ds-cpp\romfs\maps ...

pushd "%~dp0"
powershell -ExecutionPolicy Bypass -File ".\export_maps.ps1"
set ERR=%ERRORLEVEL%
popd

if %ERR% neq 0 (
    echo.
    echo Export fehlgeschlagen (Fehlercode: %ERR%)
    if not defined NO_PAUSE pause
    exit /b %ERR%
)

echo.
echo Export erfolgreich.
if not defined NO_PAUSE pause
exit /b 0
