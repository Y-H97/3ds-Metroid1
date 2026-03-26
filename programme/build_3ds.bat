@echo off
REM Zentraler Einstieg fuer den 3DS-Build mit bereits exportierten Karten.
setlocal EnableExtensions

call "%~dp0..\3ds-cpp\build_3ds.bat" %*
exit /b %ERRORLEVEL%