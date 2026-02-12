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
