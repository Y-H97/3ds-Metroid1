@echo off
pushd "%~dp0"
python "check_wiki_links.py" %*
set rc=%ERRORLEVEL%
popd
exit /b %rc%
