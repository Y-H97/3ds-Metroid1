# Workspace-Struktur

## Top-Level
- `3ds-cpp/` – C++-Game-Runtime (3DS Build + Assets + Quellcode)
- `simulator/` – Desktop-Simulatoren (Window/Console)
- `LevelEditor/` – Lua/LÖVE-Editor + Export
- `docs/` – Bereichsdokumentation (Game, Simulator, LevelEditor, Tooling)
- `DesingDokumente/` – zusätzliche Design-/Migrationsnotizen

## Datenfluss
- Editor-Quellen: `LevelEditor/level/*.lua`
- Exportziel: `3ds-cpp/romfs/maps/*.json`
- Nutzung: Runtime (`3ds-cpp/source`) und Simulator (`simulator/`)
