# Simulator Setup (Windows & Linux)

## Unterstützte Modi
- **Window-Simulator (`window_main_win32.cpp`)**
  - Plattform: **Windows** (Win32 API)
- **Console-Simulator (`main.cpp`)**
  - Plattform: **Windows + Linux**

## Abhängigkeiten
- C++17 Compiler (`g++` empfohlen)
- Für Windows: MinGW/WinLibs oder MSVC
- Für Linux: `g++`, `make` (optional)

## Windows

### 1) Build/Run Window-Simulator
- Build-only:
  - `simulator\\run_simulator_window.bat --build-only`
- Start:
  - `simulator\\run_simulator_window.bat`

### 2) Build/Run Console-Simulator
- Build-only:
  - `simulator\\run_simulator_console.bat --build-only`
- Start:
  - `simulator\\run_simulator_console.bat`

## Linux

### 1) Build/Run Console-Simulator
- Build-only:
  - `./simulator/run_simulator_console.sh --build-only`
- Start:
  - `./simulator/run_simulator_console.sh`

Falls das Script nicht ausführbar ist:
- `chmod +x simulator/run_simulator_console.sh`

### 2) Window-Simulator auf Linux
- `./simulator/run_simulator_window.sh` gibt bewusst einen Hinweis aus.
- Grund: aktueller Window-Simulator nutzt Win32 und ist nicht Linux-portabel.

Optional ausführbar machen:
- `chmod +x simulator/run_simulator_window.sh`

## Save-Pfade
- Standard: `%LOCALAPPDATA%/MetroidDesktopSimulator/sim_saves` (Windows)
- Override (Windows-Launcher): `METROID_SAVE_ROOT`

## Build-Artefakte
- `3ds-cpp/build/simulator/metroid_sim_window.exe`
- `3ds-cpp/build/simulator/metroid_sim_console.exe` (Windows)
- `3ds-cpp/build/simulator/metroid_sim_console` (Linux)

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/simulator/README|Simulator Überblick]]
- [[docs/simulator/CONTROLS|Simulator Steuerung]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
- [[docs/game/SETUP|Game Setup]]
