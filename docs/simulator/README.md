# Simulator

Dieser Bereich dokumentiert die Desktop-Simulatoren im Root-Ordner `simulator/`.

## Komponenten
- `simulator/window_main_win32.cpp`
  - Window-Simulator mit MainMenu + Gameplay, Top/Bottom-Screen-Layout, Persistenz und Input-Mapping.
- `simulator/main.cpp`
  - Console-Simulator für schnelle Core-Tests ohne Fenster-Rendering.
- `simulator/compat/desktop_3ds_compat.h`
  - Desktop-Ersatztypen/Key-Masks (`u32`, `touchPosition`, KEY_*), damit gemeinsame Menü-/Inputlogik kompiliert.

## Detaillierte Doku
- `SETUP.md` – ausführliches Setup für Windows/Linux.
- `CONTROLS.md` – vollständige Steuerung für Window- und Console-Simulator.

## Launcher
- `simulator/run_simulator_window.bat`
  - Build + Start des Window-Simulators.
  - Optional: `--build-only`
  - Optional: `METROID_SAVE_ROOT` setzen.
- `simulator/run_simulator_console.bat`
  - Build + Start des Console-Simulators.
  - Optional: `--build-only`

## Legacy-Kompatibilität
- `3ds-cpp/run_simulator_window.bat` leitet auf den Root-Launcher weiter.

## Steuerung (Window-Simulator)
- Top-Screen Bewegung: `Pfeiltasten`
- Springen: `SPACE`
- Bottom/DPad: `W/A/S/D` (zusätzlich `Q/E`, `I/K`)
- Menü bestätigen: `Enter`
- Menü zurück: `Backspace`
- Zurück ins Hauptmenü: `ESC` (Select)
- Touch-Ersatz: Linksklick im Bottom-Screen

## Persistenz (Window-Simulator)
- Standard-Speicherpfad: `%LOCALAPPDATA%/MetroidDesktopSimulator/sim_saves`
- Override möglich über `METROID_SAVE_ROOT`.

## Build-Ausgaben
- `3ds-cpp/build/simulator/metroid_sim_window.exe`
- `3ds-cpp/build/simulator/metroid_sim_console.exe`

## Muss-Checkliste vor Abschluss
- [ ] Antworten/Änderungstexte auf Deutsch
- [ ] Neuer/geänderter Code ausführlich auf Deutsch kommentiert
- [ ] Betroffene Doku angepasst
- [ ] Obsidian-Querverweise ergänzt
- [ ] Build/Test kurz validiert oder begründet ausgelassen

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/simulator/CONTROLS|Simulator Steuerung]]
- [[docs/game/RUNTIME_FLOW|Game-Laufzeitfluss]]
- [[docs/level-editor/SETUP|LevelEditor Setup]]
