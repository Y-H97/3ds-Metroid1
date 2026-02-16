@echo off
REM Baut und startet den grafischen Win32-Simulator (Windows).
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
    "%SIM_ROOT%\window_main_win32.cpp" ^
    "%CPP_ROOT%\source\core\game_core.cpp" ^
    "%CPP_ROOT%\source\core\world_map.cpp" ^
    "%CPP_ROOT%\source\menu\controllers\main_menu_controller.cpp" ^
    "%CPP_ROOT%\source\menu\views\manual_content.cpp" ^
    -lgdi32 -lwinmm
) else (
  "%CXX%" /nologo /std:c++17 /O2 /EHsc /DDESKTOP_SIMULATOR /Fe:"%OUT%" ^
    "%SIM_ROOT%\window_main_win32.cpp" ^
    "%CPP_ROOT%\source\core\game_core.cpp" ^
    "%CPP_ROOT%\source\core\world_map.cpp" ^
    "%CPP_ROOT%\source\menu\controllers\main_menu_controller.cpp" ^
    "%CPP_ROOT%\source\menu\views\manual_content.cpp" ^
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

set "METROID_MAPS_ROOT=%CPP_ROOT%\romfs\maps"
REM Optional: eigenes Save-Verzeichnis setzen (sonst %LOCALAPPDATA%\MetroidDesktopSimulator\sim_saves)
REM set "METROID_SAVE_ROOT=%CPP_ROOT%\sim_saves"

if "%~1"=="" (
  "%OUT%"
) else (
  "%OUT%" %*
)
set "SIM_EXIT=%ERRORLEVEL%"

popd
exit /b %SIM_EXIT%
