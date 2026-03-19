# Programme und Startskripte

Dieses Dokument listet die ausführbaren Programme/Skripte im Workspace und deren Zweck.

## 1) 3DS Runtime (`3ds-cpp/`)

- `3ds-cpp/source/main.cpp`
  - 3DS-Hauptprogramm (MainMenu + Gameplay).
- `3ds-cpp/Makefile`
  - Build der 3DS-Binaries (`.3dsx`, `.smdh`).
- `3ds-cpp/build_local.bat`
  - Lokales Build-Helferskript für die 3DS-Version.

## 2) Desktop-Simulator (`simulator/`)

- `simulator/run_simulator_window.bat`
  - Build + Start des Window-Simulators.
- `simulator/window_main_win32.cpp`
  - Win32-Desktop-Simulator (Top/Bottom-Screen, Menü, Gameplay).
- `simulator/run_simulator_console.bat`
  - Build + Start des Console-Simulators.
- `simulator/run_simulator_console.sh`
  - Linux Build + Start des Console-Simulators.
- `simulator/run_simulator_window.sh`
  - Linux-Hinweis (Window-Simulator aktuell nicht Linux-portabel).
- `simulator/main.cpp`
  - Konsolenbasierter Core-Simulator.
- `simulator/compat/desktop_3ds_compat.h`
  - 3DS-kompatible Key-/Touch-Typen für Desktop-Builds.

## 3) Legacy-Wrapper

- `3ds-cpp/run_simulator_window.bat`
  - Weiterleitung auf `simulator/run_simulator_window.bat` (Backwards Compatibility).

## 4) LevelEditor (`LevelEditor/`)

- `LevelEditor/run_editor.bat`
  - Startet den Editor (LÖVE).
- `LevelEditor/run_export_maps.bat`
  - Startet den Kartenexport.
- `LevelEditor/export_maps.ps1`
  - Exportiert `LevelEditor/level/*.lua` nach `3ds-cpp/romfs/maps/*.json`.
- `LevelEditor/export_maps.py`
  - Plattformübergreifender Export (`Windows/Linux`).
- `LevelEditor/run_editor.sh`
  - Startet den Editor auf Linux (`love`).
- `LevelEditor/run_export_maps.sh`
  - Startet den Python-Export auf Linux.
- `LevelEditor/main.lua`
  - Haupteinstieg des Editors.

## 5) Typischer Workflow

1. Level im `LevelEditor` bearbeiten.
2. Mit `run_export_maps.bat` in `3ds-cpp/romfs/maps` exportieren.
3. Testen im `simulator/run_simulator_window.bat`.
4. 3DS-Build über `3ds-cpp/Makefile`.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
- [[docs/workspace/README|Workspace-Struktur]]
- [[docs/simulator/README|Simulator Überblick]]
- [[docs/level-editor/README|LevelEditor Überblick]]
