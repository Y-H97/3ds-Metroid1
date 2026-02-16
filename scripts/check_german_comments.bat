@echo off
rem Batch wrapper — pushd mappt UNC‑Pfad temporär falls nötig und führt das Skript aus
pushd "%~dp0"
python "scripts\check_german_comments.py" %*
set rc=%ERRORLEVEL%
popd
exit /b %rc%