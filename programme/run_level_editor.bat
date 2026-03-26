@echo off
REM Zentraler Einstieg fuer den Windows-LevelEditor.
setlocal EnableExtensions

call "%~dp0..\LevelEditor\run_editor.bat" %*
exit /b %ERRORLEVEL%