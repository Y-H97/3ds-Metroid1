# Quick Start – Entwickler

## Ziel (5 Minuten)
- Code ändern
- im Simulator testen
- optional 3DS-Build anstoßen

## Schritte
1. Workspace öffnen: Projektroot `3ds Metroid`.
2. Window-Simulator bauen:
   - `simulator\run_simulator_window.bat --build-only`
3. Simulator starten:
   - `simulator\run_simulator_window.bat`
4. C++-Änderung in `3ds-cpp/source` machen.
5. Simulator erneut starten und Verhalten prüfen.

## Optional: 3DS Build
- In `3ds-cpp`:
  - `make`
- oder Full-Local-Workflow:
  - `3ds-cpp\build_local.bat`

## Typische Dateien
- Runtime-Entry: `3ds-cpp/source/main.cpp`
- Gameplay: `3ds-cpp/source/gameplay/*`
- Core: `3ds-cpp/source/core/*`
- Simulator: `simulator/window_main_win32.cpp`

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/game/README|Game Überblick]]
- [[docs/game/SETUP|Game Setup]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
