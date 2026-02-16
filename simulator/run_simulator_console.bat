@echo off
REM Baut und startet den textbasierten Console-Simulator (Windows).
setlocal EnableExtensions

set "SIM_ROOT=%~dp0"
if "%SIM_ROOT:~-1%"=="\" set "SIM_ROOT=%SIM_ROOT:~0,-1%"
set "PROJECT_ROOT=%SIM_ROOT%\.."
set "CPP_ROOT=%PROJECT_ROOT%\3ds-cpp"

set "BUILD_ONLY=0"
if /I "%~1"=="--build-only" (
  set "BUILD_ONLY=1"
  shift
)

pushd "%CPP_ROOT%"

set "CXX="
where g++ >nul 2>&1
if not errorlevel 1 set "CXX=g++"
if not defined CXX if exist "C:\msys64\ucrt64\bin\g++.exe" set "CXX=C:\msys64\ucrt64\bin\g++.exe"
if not defined CXX if exist "C:\msys64\mingw64\bin\g++.exe" set "CXX=C:\msys64\mingw64\bin\g++.exe"
if not defined CXX (
  for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs*") do (
    if exist "%%~fD\mingw64\bin\g++.exe" (
      set "CXX=%%~fD\mingw64\bin\g++.exe"
      goto :compiler_found
    )
  )
)

if not defined CXX (
  echo FEHLER: Kein g++ gefunden.
  popd
  exit /b 1
)

:compiler_found

if not exist "build\simulator" mkdir "build\simulator"
set "OUT=build\simulator\metroid_sim_console.exe"

echo Baue Console-Simulator mit %CXX%...
"%CXX%" -std=gnu++17 -O2 -Wall -Wextra -o "%OUT%" ^
  "%SIM_ROOT%\main.cpp" ^
  "%CPP_ROOT%\source\core\game_core.cpp"
if errorlevel 1 (
  echo FEHLER: Build fehlgeschlagen.
  popd
  exit /b 1
)

if "%BUILD_ONLY%"=="1" (
  echo Build erfolgreich: "%OUT%"
  popd
  exit /b 0
)

echo.
echo Starte Console-Simulator...
echo.
if "%~1"=="" (
  "%OUT%" "%CPP_ROOT%\romfs\maps\test.json"
) else (
  "%OUT%" %*
)
set "SIM_EXIT=%ERRORLEVEL%"

popd
exit /b %SIM_EXIT%
