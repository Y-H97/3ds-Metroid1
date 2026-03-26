# Tooling, Build und Skripte

## Zentraler Einstieg
- `programme/README.md`
  - Beschreibt den zentralen Programme-Ordner und die bevorzugten Startpunkte.
- `programme/build_game_from_editor.bat`
  - Standardablauf fuer den Nintendo-3DS-Build: exportiert zuerst die Level und startet danach den Build.
- `programme/build_3ds.bat`
  - Zentraler Einstieg fuer den reinen 3DS-Build.
- `programme/export_level_maps.bat`
  - Zentraler Einstieg fuer den Kartenexport.
- `programme/run_level_editor.bat`
  - Zentraler Einstieg fuer den Windows-LevelEditor.
- `programme/run_simulator_window.bat`
  - Zentraler Einstieg fuer den grafischen Windows-Simulator.

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

Diese Artefakte sind generierte Build-Ausgaben und sollen nicht als gepflegte
Quellstände im Repository liegen. Für lokale Builds werden sie durch die
`.gitignore` ausgeblendet. Das gleiche gilt für lokale Simulator-Speicherstände
unter `3ds-cpp/sim_saves/`.

## Typischer End-to-End-Workflow
1. `programme/run_level_editor.bat`
2. `programme/export_level_maps.bat`
3. `programme/run_simulator_window.bat`
4. `programme/build_game_from_editor.bat`

## Legacy-Einstiege
- Die Skripte in `LevelEditor/`, `simulator/` und `3ds-cpp/` bleiben aus Kompatibilitaetsgruenden erhalten.
- Neue Dokumentation und neue Workflows sollen jedoch immer den Ordner `programme/` referenzieren.

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
