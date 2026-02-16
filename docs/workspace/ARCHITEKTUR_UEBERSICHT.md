# Architektur-Übersicht (einfach)

Diese Seite zeigt den technischen Gesamtfluss in **kompakter Form**.

## Systembild

```mermaid
flowchart LR
    A[LevelEditor UI<br/>main.lua + views.lua] --> B[Leveldaten .lua<br/>LevelEditor/level/*.lua]
    B --> C[Export<br/>export_maps.py / export_maps.ps1 / map_exporter.lua]
    C --> D[JSON-Maps<br/>3ds-cpp/romfs/maps/*.json]

    D --> E[Game Runtime (3DS)<br/>3ds-cpp/source]
    D --> F[Desktop Simulator<br/>simulator/window_main_win32.cpp + simulator/main.cpp]

    E --> G[Save/Settings 3DS]
    F --> H[Save/Settings Desktop]
```

## Kurz erklärt
- **LevelEditor** erzeugt und bearbeitet Räume als Lua-Tabellen.
- **Export** konvertiert diese Lua-Daten in das JSON-Format, das Runtime und Simulator lesen.
- **Game Runtime (3DS)** enthält Spielzustand, Physik, Menü und Rendering.
- **Desktop Simulator** nutzt dieselbe Kernlogik (insb. `GameCore`) für schnelles Testen am PC.
- **Save/Settings** werden je Umgebung getrennt gespeichert (3DS vs. Desktop).

## Modul-Fokus
- **Core-Logik**: `3ds-cpp/source/core`
- **Gameplay-Orchestrierung**: `3ds-cpp/source/gameplay`
- **Menüsystem**: `3ds-cpp/source/menu`
- **Editor-Workflows**: `LevelEditor/*.lua`
- **Export-Pipeline**: `LevelEditor/export_maps.py`, `LevelEditor/export_maps.ps1`, `LevelEditor/map_exporter.lua`

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/workspace/EINSTEIGER_GUIDE|Einsteiger-Guide]]
- [[docs/game/ARCHITECTURE|Game-Architektur]]
- [[docs/level-editor/README|LevelEditor Überblick]]
- [[docs/simulator/README|Simulator Überblick]]
