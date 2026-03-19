# Tooling, Build und Skripte

## 3DS-Build
- `3ds-cpp/Makefile`
  - devkitARM/libctru-basierter Build (`.3dsx`, `.smdh`, RomFS, gfx conversion).
- `3ds-cpp/build_local.bat`
  - Lokaler Build-Workflow:
    1. Export LevelEditor-Maps
    2. Spiegeln nach `%LOCALAPPDATA%\Temp\3ds-cpp-build`
    3. `make clean` + `make`
    4. Artefakte zurückkopieren

## Simulator-Build
- `simulator/run_simulator_window.bat`
  - Baut den Win32-Simulator gegen Quellen aus `3ds-cpp/source`.
- `simulator/run_simulator_console.bat`
  - Baut den Console-Simulator.
- `simulator/run_simulator_console.sh`
  - Linux Build/Run für den Console-Simulator.
- `simulator/run_simulator_window.sh`
  - Linux-Hinweisskript (Window-Simulator ist Win32-only).

## LevelEditor Export (plattformübergreifend)
- `LevelEditor/export_maps.py`
  - Cross-Platform Export/Validierung.
- `LevelEditor/run_export_maps.sh`
  - Linux-Wrapper für den Python-Export.

## Artefakte
- 3DS:
  - `3ds-cpp/3ds-cpp.3dsx`
  - `3ds-cpp/3ds-cpp.smdh`
- Simulator:
  - `3ds-cpp/build/simulator/metroid_sim_window.exe`
  - `3ds-cpp/build/simulator/metroid_sim_console.exe`

## Typischer End-to-End-Workflow
1. `LevelEditor/run_editor.bat`
2. `LevelEditor/run_export_maps.bat`
3. `simulator/run_simulator_window.bat`
4. `3ds-cpp/build_local.bat` oder direkt `make` in `3ds-cpp`

## Muss-Checkliste vor Abschluss
- [ ] Antworten/Änderungstexte auf Deutsch
- [ ] Neuer/geänderter Code ausführlich auf Deutsch kommentiert
- [ ] Betroffene Doku angepasst
- [ ] Obsidian-Querverweise ergänzt
- [ ] Build/Test kurz validiert oder begründet ausgelassen

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/PROGRAMS|Programme und Startskripte]]
- [[docs/tooling/CHANGELOG_UND_VERSIONIERUNG|Changelog und Versionierung]]
- [[docs/game/SETUP|Game Setup]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/level-editor/SETUP|LevelEditor Setup]]
