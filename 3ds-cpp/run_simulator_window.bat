@echo off
setlocal EnableExtensions

set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"

set "BUILD_ONLY=0"
if /I "%~1"=="--build-only" (
  set "BUILD_ONLY=1"
  shift
)

pushd "%ROOT%"

set "CXX="
set "CXX_MODE="

where g++ >nul 2>&1
if not errorlevel 1 (
  set "CXX=g++"
  set "CXX_MODE=gcc_like"
)

if not defined CXX (
  if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "CXX=C:\msys64\ucrt64\bin\g++.exe"
    set "CXX_MODE=gcc_like"
  )
)

if not defined CXX (
  if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "CXX=C:\msys64\mingw64\bin\g++.exe"
    set "CXX_MODE=gcc_like"
  )
)

if not defined CXX (
  for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs*") do (
    if exist "%%~fD\mingw64\bin\g++.exe" (
      set "CXX=%%~fD\mingw64\bin\g++.exe"
      set "CXX_MODE=gcc_like"
      goto :compiler_found
    )
  )
)

if not defined CXX (
  where cl >nul 2>&1
  if not errorlevel 1 (
    set "CXX=cl"
    set "CXX_MODE=msvc"
  )
)

if not defined CXX (
  echo FEHLER: Kein C++-Compiler gefunden.
  echo Erwartet wird einer von: g++, cl.exe
  echo Installiere z. B. WinLibs/MSYS2 oder Visual Studio Build Tools.
  popd
  exit /b 1
)

:compiler_found

if not exist "build\simulator" mkdir "build\simulator"

set "OUT=build\simulator\metroid_sim_window.exe"

taskkill /IM metroid_sim_window.exe /F >nul 2>&1
taskkill /IM metroid_sim_window_new.exe /F >nul 2>&1

echo Baue Window-Simulator mit %CXX%...

if "%CXX_MODE%"=="gcc_like" (
  "%CXX%" -std=gnu++17 -O2 -Wall -Wextra -static -static-libgcc -static-libstdc++ -mwindows -DDESKTOP_SIMULATOR -o "%OUT%" ^
    "%ROOT%\simulator\window_main_win32.cpp" ^
    "%ROOT%\source\core\game_core.cpp" ^
    "%ROOT%\source\core\world_map.cpp" ^
    "%ROOT%\source\menu\controllers\main_menu_controller.cpp" ^
    "%ROOT%\source\menu\views\manual_content.cpp" ^
    -lgdi32 -lwinmm
) else (
  "%CXX%" /nologo /std:c++17 /O2 /EHsc /DDESKTOP_SIMULATOR /Fe:"%OUT%" ^
    "%ROOT%\simulator\window_main_win32.cpp" ^
    "%ROOT%\source\core\game_core.cpp" ^
    "%ROOT%\source\core\world_map.cpp" ^
    "%ROOT%\source\menu\controllers\main_menu_controller.cpp" ^
    "%ROOT%\source\menu\views\manual_content.cpp" ^
    gdi32.lib user32.lib winmm.lib
)

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
echo Starte Window-Simulator...
echo Steuerung: A/D oder Pfeile, SPACE, Linksklick, R, ESC
echo.

set "METROID_MAPS_ROOT=%ROOT%\romfs\maps"
REM Optional: eigenes Save-Verzeichnis setzen (sonst %LOCALAPPDATA%\MetroidDesktopSimulator\sim_saves)
REM set "METROID_SAVE_ROOT=%ROOT%\sim_saves"

if "%~1"=="" (
  "%OUT%"
) else (
  "%OUT%" %*
)
set "SIM_EXIT=%ERRORLEVEL%"

popd
exit /b %SIM_EXIT%
