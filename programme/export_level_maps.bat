@echo off
REM Zentraler Einstieg fuer den Kartenexport aus dem LevelEditor.
setlocal EnableExtensions

call "%~dp0..\LevelEditor\run_export_maps.bat" %*
exit /b %ERRORLEVEL%