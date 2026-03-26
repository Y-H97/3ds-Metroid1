@echo off
REM Zentraler Einstieg fuer den grafischen Windows-Simulator.
setlocal EnableExtensions

call "%~dp0..\simulator\run_simulator_window.bat" %*
exit /b %ERRORLEVEL%