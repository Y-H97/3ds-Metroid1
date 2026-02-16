# Refactoring-Bewertung (Stand: 2026-02-13)

## Kürzlich abgeschlossen
- Handbuchdaten aus `main_menu_view.cpp` in `manual_content.*` ausgelagert.
- `gameplay_scene` in Verantwortungsbereiche getrennt:
  - `gameplay_scene.cpp` (Kernfluss/Update/Render)
  - `gameplay_scene_input.cpp` (Input + UI-Interaktion)
  - `gameplay_scene_persistence.cpp` (Save/Visited/Slot-Flow)
- Bottom-UI je Tab modularisiert:
  - `bottom_ui_map.cpp`
  - `bottom_ui_inventory.cpp`
  - `bottom_ui_settings.cpp`
  - `bottom_ui_debug.cpp`
  - `bottom_ui.cpp` nur Dispatcher + Tab-Leiste
- `main_menu_controller.cpp` intern pro Menü-State in Key/Touch-Handler aufgeteilt.
- `main_menu_view.cpp` Top-Screen-Rendering in kleinere state-spezifische Helfer zerlegt.

## Aktuelle Hotspots (optional)

### 1) Gameplay-Transitions (niedrig-mittel)
**Warum:** Übergangslogik zwischen Räumen ist weiterhin dicht konzentriert.

**Möglicher Schritt:**
- Optional `gameplay_scene_transitions.cpp` für Raumwechsel/Übergangsberechnung.

### 2) UI-Layer-Konstanten (niedrig)
**Warum:** Farb- und Layoutwerte sind über mehrere Views/Renderer verteilt.

**Möglicher Schritt:**
- Gemeinsames UI-Theme/Layout-Header mit benannten Konstanten.

## Derzeit stabil / nicht dringend
- `main.cpp` ist kompakt als App-Orchestrator.
- `app_settings.*` bleibt klein und klar.
- Menü-Controller und Menü-View sind nun gut wartbar getrennt.

## Empfohlene Reihenfolge für weitere Refactors
1. Optional: Gameplay-Transitions auslagern.
2. Optional: UI-Konstanten zentralisieren.
3. Danach nur noch bei konkretem Feature-Druck weiter aufteilen.
