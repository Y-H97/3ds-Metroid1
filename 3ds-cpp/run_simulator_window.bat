@echo off
REM Dieses Skript startet den Windows-Simulator über den Launcher im /simulator-Ordner.
REM Falls beim Doppelklick ein Dialog zur Programmauswahl erscheint,
REM sind .bat-Dateien auf Deinem System nicht korrekt mit cmd.exe verknüpft.
REM LEGACY-HINWEIS: Bevorzugter Einstieg ist jetzt ..\programme\run_simulator_window.bat.
REM Führe in diesem Fall das Skript "scripts\fix_bat_assoc.ps1" als Administrator aus
REM oder starte diesen Batch über eine Eingabeaufforderung (cmd.exe).
REM
REM WICHTIG für PowerShell-Benutzer:
REM PowerShell führt standardmäßig keine Programme aus dem aktuellen
REM Verzeichnis aus. Verwende also
REM     .\run_simulator_window.bat
REM sonst bekommst du die Meldung "CommandNotFoundException".
setlocal

set "FORWARD=%~dp0..\simulator\run_simulator_window.bat"
if not exist "%FORWARD%" (
  echo FEHLER: Simulator-Launcher nicht gefunden: "%FORWARD%"
  exit /b 1
)

call "%FORWARD%" %*
exit /b %ERRORLEVEL%
