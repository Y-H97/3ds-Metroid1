@echo off
REM Lokaler Build-Workflow: exportiert Maps, spiegelt Projekt in Temp,
REM baut dort und kopiert Artefakte zurück ins Workspace.
setlocal EnableExtensions

set "SRC=%~dp0"
if "%SRC:~-1%"=="\" set "SRC=%SRC:~0,-1%"

set "DST=%LOCALAPPDATA%\Temp\3ds-cpp-build"

echo [0/4] Export LevelEditor maps...
if exist "%SRC%\..\LevelEditor\run_export_maps.bat" (
  call "%SRC%\..\LevelEditor\run_export_maps.bat" --no-pause
  if errorlevel 1 (
    echo ERROR: Map export failed
    exit /b 1
  )
) else (
  echo ERROR: Missing "%SRC%\..\LevelEditor\run_export_maps.bat"
  exit /b 1
)

echo [1/4] Prepare local build folder...
if exist "%DST%" rmdir /S /Q "%DST%"
mkdir "%DST%" >nul 2>&1
if errorlevel 1 (
  echo ERROR: Could not create "%DST%"
  exit /b 1
)

echo [2/4] Mirror project to local path...
robocopy "%SRC%" "%DST%" /MIR /NFL /NDL /NJH /NJS /NP >nul
set "RC=%ERRORLEVEL%"
if %RC% GEQ 8 (
  echo ERROR: Robocopy failed with code %RC%
  exit /b %RC%
)

echo [3/4] Build in local path...
pushd "%DST%"
call make clean
if errorlevel 1 (
  echo ERROR: make clean failed
  popd
  exit /b 1
)
call make
if errorlevel 1 (
  echo ERROR: make failed
  popd
  exit /b 1
)
popd

echo [4/4] Copy artifacts back to workspace...
if exist "%DST%\3ds-cpp-build.3dsx" (
  copy /Y "%DST%\3ds-cpp-build.3dsx" "%SRC%\3ds-cpp.3dsx" >nul
) else (
  echo ERROR: Missing output "%DST%\3ds-cpp-build.3dsx"
  exit /b 1
)

if exist "%DST%\3ds-cpp-build.smdh" (
  copy /Y "%DST%\3ds-cpp-build.smdh" "%SRC%\3ds-cpp.smdh" >nul
) else (
  echo WARNING: Missing output "%DST%\3ds-cpp-build.smdh"
)

echo.
echo Build done successfully.
echo Output: "%SRC%\3ds-cpp.3dsx"
echo         "%SRC%\3ds-cpp.smdh"
exit /b 0
