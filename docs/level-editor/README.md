# LevelEditor

Dieser Bereich dokumentiert den Lua/LÖVE-basierten Level-Editor in `LevelEditor/`.

## Detaillierte Doku
- `SETUP.md` – Setup und Export unter Windows/Linux.

## Einstiegspunkte
- `LevelEditor/main.lua`
  - Hauptprogramm des Editors.
- `LevelEditor/conf.lua`
  - LÖVE-Konfiguration.

## Start- und Export-Skripte
- `LevelEditor/run_editor.bat`
  - Startet den Editor über festen `love.exe`-Pfad.
  - Nutzt `pushd` für UNC/Netzwerkpfade.
- `LevelEditor/run_export_maps.bat`
  - Führt den Kartenexport aus (`export_maps.ps1`).
  - Unterstützt `--no-pause`.
- `LevelEditor/export_maps.ps1`
  - Konvertiert `level/*.lua` nach `3ds-cpp/romfs/maps/*.json`.
  - Validiert Raumdaten (Breite/Höhe/Tiles) und Weltreferenzen.

## Interne Module
- `constants.lua` – zentrale Konstanten.
- `state.lua` – Editorzustand.
- `views.lua` – Rendering/Views.
- `ui_components.lua` – UI-Bausteine.
- `editor_actions.lua` – Bearbeitungsaktionen.
- `io_utils.lua` – Ein-/Ausgabehilfen.
- `map_exporter.lua` – Exportlogik im Editor-Kontext.

## Datenquellen
- `LevelEditor/level/*.lua`
  - Raumdateien + `world.lua` als Weltlayout.

## Exportziel
- `3ds-cpp/romfs/maps/*.json`
  - Diese JSON-Dateien werden von Runtime und Simulator geladen.

## Hinweis
- `run_editor.bat` enthält aktuell einen festen LÖVE-Pfad (`C:\Program Files\LOVE\love.exe`).
  Bei abweichender Installation muss dieser Pfad angepasst werden.
