# Metroidvania 3DS C++ Core

Dieses Projekt enthält den C++‑Core für die 3DS‑Version (libctru + citro2d/citro3d) und einen
PC‑Test‑Build auf SDL2, damit die Core‑Logik schnell unter Linux getestet werden kann.

## 3DS Build (devkitARM)
Voraussetzungen:
- devkitARM + libctru
- citro2d / citro3d

Build:
```
make
```

Ausgabe: `<projektname>.3dsx`

## PC‑Test Build (SDL2, Linux)
Voraussetzungen:
- SDL2 (dev Paket)
- CMake

Build:
```
cd ../pc-sdl
mkdir -p build
cd build
cmake ..
make
```

Starten (optional mit Map‑Pfad):
```
./metroidvania_pc ../3ds-cpp/romfs/maps
```

## Windows Console‑Simulator (ohne 3DS‑Emulator)
Der Simulator nutzt direkt `GameCore` (Physik/Kollision/Map‑Logik) und läuft in der Konsole.

Voraussetzung:
- `g++` im `PATH` (z. B. über MSYS2/MinGW-w64)

Build + Start:
```
run_simulator.bat
```

Optional mit bestimmter Map:
```
run_simulator.bat romfs/maps/0.json
```

Wichtige Befehle im Simulator:
- `step [n] [keys]` (`keys`: `l`, `r`, `j`)
- `hold <l|r|j> <on|off>`
- `view [w h]`
- `map <pfad>`
- `status`, `reset`, `quit`

## Windows Fenster‑Simulator (Tastatur + Maus)
Für schnelles Testen mit Spiel-Fenster statt Konsole.

Start:
```
run_simulator_window.bat
```

Optional mit Map:
```
run_simulator_window.bat romfs/maps/0.json
```

Steuerung:
- **Top-Screen (CPad):** `Pfeile` (nur Top), `SPACE` springen
- **Bottom-Screen (DPad):** `W/A/S/D` (oder `Q/E` für links/rechts, `I/K` für hoch/runter)
- **Menü (Bottom-Screen):** `W/A/S/D` + `Enter` (=A/Y) + `Backspace` (=B) + `Linksklick`
- **Gameplay Navigation:** `ESC` zurück ins Hauptmenü (wie `Select`)
- **Bottom-Tabs im Gameplay:** `A/D` oder `Q/E` (oder Klick auf Tab-Leiste)
- **Settings-Tab:** `W/S` oder `I/K`, `Enter` bestätigen

## Migrations‑Plan (Kurzfassung)
1. **Core‑Module** in C++ bauen: Input, Player‑Movement, Collision, Map‑Loading.
2. **Rendering/Audio** plattformabhängig: 3DS (citro2d/3d) und PC (SDL2) getrennt halten.
3. **Datenformate** stabilisieren: Level‑JSON aus dem Editor unverändert nutzen.
4. **Feature‑Parity** pro Modul:
	- Player (Movement, Collision)
	- World (Level‑Streaming, Rooms)
	- UI (HUD, Menüs)
	- Systems (Settings, Logger, Checkpoints)
5. **Optimierungen**: Chunking, Culling, Sprite‑Batching, feste Zeitschritte.
6. **Regression‑Tests** auf PC: Level‑Loads, Transitions, Physics, Inputs.
7. **Finaler 3DS‑Test** auf Hardware.

## Verzeichnisstruktur
- `source/` 3DS‑Entry + Rendering
- `source/core/` Plattform‑unabhängiger Core (Input/Physics/Map)
- `../pc-sdl/` PC‑Test‑Build mit SDL2
