@echo off
echo Exportiere LevelEditor Maps nach 3ds-cpp\romfs\maps ...

pushd "%~dp0"
powershell -ExecutionPolicy Bypass -File ".\export_maps.ps1"
set ERR=%ERRORLEVEL%
popd

if %ERR% neq 0 (
    echo.
    echo Export fehlgeschlagen (Fehlercode: %ERR%)
    pause
    exit /b %ERR%
)

echo.
echo Export erfolgreich.
pause
exit /b 0
