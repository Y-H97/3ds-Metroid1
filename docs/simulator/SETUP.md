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

### Voraussetzungen
Der Launcher benötigt einen C++17-fähigen Compiler auf der PATH:
- **g++** (MinGW/MSYS2, WinLibs, WSL etc.) oder
- **cl.exe** (Visual Studio Build Tools/VS Community).  

Falls beim Aufruf folgende Fehlermeldung erscheint:
```
FEHLER: Kein C++-Compiler gefunden.
Erwartet wird einer von: g++, cl.exe
Installiere z. B. WinLibs/MSYS2 oder Visual Studio Build Tools.
```
dann ist entweder kein Compiler installiert oder er ist nicht in
des PATH aufgenommen.

#### Compiler installieren
1. **MSYS2** (empfohlen für MinGW-Windows):
   - https://www.msys2.org/
   - Nach Installation: `pacman -S mingw-w64-x86_64-toolchain`
   - Füge `C:\msys64\mingw64\bin` (oder `ucrt64`) zum PATH hinzu.
   - Hinweis: Der Launcher sucht automatisch in `C:\msys64\ucrt64\bin`.
2. **WinLibs** (portable GCC-Builds):
   - https://winlibs.com/
   - Entpacke das Archiv, z.B. in `"%USERPROFILE%\Downloads"`.
   - Der Batch startet nun auch automatisch, wenn Dein Download-Ordner
     eine `winlibs-…`-Unterverzeichnis enthält und dort `mingw64\bin\g++.exe`.
   - Alternativ kannst Du das `bin`-Verzeichnis manuell dem PATH hinzufügen.
3. **Visual Studio Build Tools**
   - https://aka.ms/vs/17/release/vs_BuildTools.exe
   - Nach Installation: starte die Entwickler-Eingabeaufforderung oder
     füge `cl.exe` zum globalen PATH.

### 1) Build/Run Window-Simulator
- Build-only:
  - `simulator\\run_simulator_window.bat --build-only`
- Start:
  - `simulator\\run_simulator_window.bat`

> **Achtung:** Windows führt Batch-Dateien (`.bat`) standardmäßig mit
> `cmd.exe` aus. Wenn beim Doppelklicken ein Dialog zur Programmauswahl
> auftaucht, ist diese Zuordnung beschädigt.
> 
> **PowerShell:** Rufe das Skript mit `.\run_simulator_window.bat` auf –
> **ohne** den vorangestellten `PS C:\...>`-Prompt. Kopiere niemals
> den Prompt-Text in die Eingabe, sonst interpretiert PowerShell ihn als
> zusätzlichen Befehl und bricht mit einer Fehlermeldung ab.
> 
> Alternativ starte die Datei aus einer CMD‑Shell oder repariere die
> Zuordnung mittels `scripts\fix_bat_assoc.ps1` (als Administrator).

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
