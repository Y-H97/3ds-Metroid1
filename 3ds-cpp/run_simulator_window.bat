@echo off
setlocal

set "FORWARD=%~dp0..\simulator\run_simulator_window.bat"
if not exist "%FORWARD%" (
  echo FEHLER: Simulator-Launcher nicht gefunden: "%FORWARD%"
  exit /b 1
)

call "%FORWARD%" %*
exit /b %ERRORLEVEL%
