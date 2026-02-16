@echo off
pushd "%~dp0"
python "scripts\check_wiki_links.py" %*
set rc=%ERRORLEVEL%
popd
exit /b %rc%