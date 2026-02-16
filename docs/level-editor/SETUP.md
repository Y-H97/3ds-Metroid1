# LevelEditor Setup (Windows & Linux)

## Abhängigkeit
- LÖVE (love2d)

## Windows

### Editor starten
- `LevelEditor\\run_editor.bat`

Hinweis:
- Das Skript nutzt aktuell festen Pfad:
  - `C:\\Program Files\\LOVE\\love.exe`
- Bei abweichender Installation Pfad in `run_editor.bat` anpassen.

### Karten exportieren
- `LevelEditor\\run_export_maps.bat`
- ohne Pause:
  - `LevelEditor\\run_export_maps.bat --no-pause`

## Linux

### Editor starten
- `./LevelEditor/run_editor.sh`
- Voraussetzung: `love` im `PATH`

Falls das Script nicht ausführbar ist:
- `chmod +x LevelEditor/run_editor.sh`

### Karten exportieren
- `./LevelEditor/run_export_maps.sh`
- Verwendet:
  - `LevelEditor/export_maps.py` (Python 3)

Falls das Script nicht ausführbar ist:
- `chmod +x LevelEditor/run_export_maps.sh`

## Export-Pipeline
- Quelle: `LevelEditor/level/*.lua`
- Ziel: `3ds-cpp/romfs/maps/*.json`
- Weltlayout: `LevelEditor/level/world.lua` -> `world.json`

## Validierung
Beim Export werden geprüft:
- konsistente Raumzeilenbreiten (`width`, `height`, `tiles`)
- dass `world.lua` nur auf existierende Raumdateien verweist
