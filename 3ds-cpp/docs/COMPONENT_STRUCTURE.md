# Komponenten in der Ordnerstruktur

Dieses Dokument zeigt, wo welche Komponenten liegen und welche Aufgabe die Module haben.

## 1) C++ Spielruntime (`3ds-cpp/`)

### Root
- `Makefile`
  - Build-Definition für 3DS (`.3dsx`, `.smdh`).
- `README.md`
  - Projektstart, Build-Hinweise.
- `romfs/`
  - Laufzeit-Assets (u. a. Karten unter `romfs/maps`).
- `source/`
  - Gesamter C++ Quellcode.

### `source/main.cpp`
- App-Orchestrator.
- Wechselt zwischen Hauptmenü und Gameplay.
- Verbindet Menü-Aktionen (Start/Fortsetzen/Reset/Exit) mit `GameplayScene`.

### `source/core/`
- `game_core.*`
  - Kern-Gameplay (Spielerbewegung, Kollision, Raumdaten-Anbindung).
- `world_map.*`
  - Welt-/Raumstruktur, Raumwechsel-Kontext.
- `app_settings.*`
  - Persistente globale Einstellungen (z. B. Debug, Controls-Swap, FPS-Option).

### `source/menu/controllers/`
- `main_menu_controller.*`
  - Menü-Zustandsmaschine.
  - Verarbeitet Buttons/Touch pro Menü-State.
  - Liefert abstrakte `MenuAction`-Signale an `main.cpp`.

### `source/menu/views/`
- `main_menu_view.*`
  - Reines Rendering des Hauptmenüs (Top/Bottom).
  - Enthält State-spezifische Renderpfade für Home/Play/Options/Handbuch.
- `manual_content.*`
  - Handbuchdaten (Themen, Textzeilen, Scroll-Helfer).

### `source/gameplay/`
- `gameplay_scene.h`
  - Öffentliche Gameplay-API (Update, Render, Save/Load, Slot-Funktionen).
- `gameplay_scene.cpp`
  - Kernfluss: Update-Zyklus, Top-Level Render-Delegation.
- `gameplay_scene_input.cpp`
  - Input-Logik + UI-Interaktion im Gameplay.
- `gameplay_scene_persistence.cpp`
  - Savegame/Checkpoint, Fog-of-War-Visited, Slot-Persistenz.

### `source/gameplay/render/`
- `tile_renderer.*`
  - Karten-/Tile-Rendering.
- `bottom_ui.cpp`
  - Dispatcher für Bottom-Screen + Tab-Leiste.
- `bottom_ui_map.cpp`
  - Map-Tab Darstellung.
- `bottom_ui_inventory.cpp`
  - Inventar-Tab Darstellung.
- `bottom_ui_settings.cpp`
  - Settings-Tab Darstellung.
- `bottom_ui_debug.cpp`
  - Debug-Tab Darstellung.

### `source/ui/`
- `text_renderer.*`
  - Gemeinsamer Text-Render-Wrapper für UI-Ausgaben.

## 2) LevelEditor (`LevelEditor/`)

### Root
- `main.lua`
  - Einstiegspunkt der Editor-App.
- `conf.lua`
  - LÖVE-Konfiguration.
- `constants.lua`
  - Konstante Werte (Grid/Tile/UI-Basics).
- `state.lua`
  - Zentraler Editor-State.
- `views.lua`
  - View/Render-Helfer im Editor.
- `ui_components.lua`
  - Wiederverwendbare UI-Komponenten.
- `editor_actions.lua`
  - Editier-Aktionen (z. B. Platzieren/Ändern).
- `io_utils.lua`
  - Import/Export für Leveldaten.
- `run_editor.bat`
  - Startscript für lokalen Editor-Run.

### `level/`
- `*.lua`
  - Quell-Level für den Editor.
  - Werden in den C++-Runtime-Assetpfad exportiert (`3ds-cpp/romfs/maps`).

## 3) Datenfluss zwischen Editor und Runtime
- Editor bearbeitet `LevelEditor/level/*.lua`.
- Export schreibt JSON-Karten nach `3ds-cpp/romfs/maps/*.json`.
- Runtime lädt diese Karten im Spielstart-/Raumwechsel-Kontext.
