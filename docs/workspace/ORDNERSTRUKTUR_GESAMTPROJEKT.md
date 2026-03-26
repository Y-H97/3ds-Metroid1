# Ordnerstruktur Gesamtprojekt

Diese Seite beschreibt die Ordnerstruktur des Projekts und beantwortet pro Bereich:
- Was liegt in diesem Ordner?
- Wofuer wird der Ordner verwendet?
- Welche Abhaengigkeiten bestehen?

## 1) Top-Level im Repository `3ds-Metroid1`

| Ordner/Datei | Inhalt | Verwendung | Abhaengigkeiten |
|---|---|---|---|
| `.github/` | CI-Workflows, PR-Templates, Copilot-Regeln | Qualitaetssicherung und Projektprozesse | GitHub Actions, Python fuer Pruefskripte |
| `.vscode/` | Workspace-Settings und Tasks | Einheitliche lokale Workflows in VS Code | PowerShell, lokale Toolchains |
| `.obsidian/` | Obsidian-Workspace-Metadaten | Navigierbarkeit der Doku in Obsidian | Obsidian (optional) |
| `programme/` | Zentraler Einstieg fuer alle Startskripte | Gebuendelte Programme fuer Editor, Export, Simulator und 3DS-Build | Wrapper auf `LevelEditor/`, `simulator/`, `3ds-cpp/` |
| `3ds-cpp/` | 3DS-Runtime, C/C++-Quellen, ROMFS-Inhalte | Hauptspiel fuer Nintendo 3DS, zentrale Core-Logik | devkitARM, libctru, citro2d/citro3d, make |
| `LevelEditor/` | Lua/Love2D-Editor, Leveldaten, Exportskripte | Erstellen/Bearbeiten von Raeumen und Weltlayout | LÖVE (love2d), PowerShell, optional Python 3 |
| `simulator/` | Desktop-Simulatoren (Window + Console) | Schnelles Testen der Spiel-Logik am PC | C++17-Compiler (`g++` oder `cl.exe`) |
| `scripts/` | Pruef-, Installations- und Hilfsskripte | Doku-/Kommentarchecks, Setup-Unterstuetzung | PowerShell, Python 3, teils devkitPro/MSYS2 |
| `docs/` | Gesamte Projektdokumentation | Wissen, Architektur, Setup, Workflows | Markdown, Obsidian-Wiki-Links |
| `changelog/` | Versionsspezifische Aenderungsseiten | Release-Historie je Version | Keine Laufzeitabhaengigkeit |
| `DesingDokumente/` | Design- und Migrationsnotizen | Konzeptarbeit, Analyse, Planungsgrundlagen | Keine Laufzeitabhaengigkeit |
| `CHANGELOG.md` | Uebergeordnete Aenderungsuebersicht | Einstieg in Versionsaenderungen | Verweise auf `changelog/` |
| `requirements-dev.txt` | Dev-Tooling-Pakete | Lokale Entwicklungspruefungen (z. B. pre-commit) | Python + pip |

## 2) Technische Kernordner im Detail

### `programme/` (zentraler Programmeinstieg)
- `README.md`: Uebersicht ueber alle Startpunkte und deren Aufgabe.
- `run_level_editor.bat`: Leitet den Start des Editors weiter.
- `export_level_maps.bat`: Leitet den Kartenexport weiter.
- `run_simulator_window.bat`: Leitet den Simulatorstart weiter.
- `build_3ds.bat`: Leitet den 3DS-Build weiter.
- `build_game_from_editor.bat`: Fuehrt Export und 3DS-Build in einem Schritt aus.

Abhaengigkeiten:
- Nutzt die bestehenden fachlichen Skripte in `LevelEditor/`, `simulator/` und `3ds-cpp/`.
- Dient bewusst als stabile Einstiegsschicht, damit die interne Projektstruktur getrennt vom Nutzereinstieg bleiben kann.

### `3ds-cpp/` (3DS-Runtime + Core)
- `source/`: Code der Runtime.
  - `core/`: Plattformunabhaengige Kernlogik (z. B. Spielzustand, Weltlogik).
    - `game_core.*`: Runtime-Fassade fuer Spielzustand und Player-Delegation.
    - `tile_map_io.cpp`: Laedt Kartenformate und extrahiert exportierte Items.
    - `world_map.*`: Welt-/Raumverknuepfung und Spatial-Map.
  - `gameplay/`: Gameplay-spezifische Ablaufe und Rendering-Helfer.
    - `gameplay_scene.cpp`: Init, Render und gemeinsamer Szenen-Grundzustand.
    - `gameplay_scene_runtime.cpp`: Frame-Update-Orchestrierung.
    - `gameplay_scene_transition.cpp`: Raumwechsel, Respawn und Grid-Aktualisierung.
    - `gameplay_scene_items.cpp`: Map-Items, Pickup-Kollisionen und Meldungen.
    - `gameplay_scene_input.cpp`: Rohinput und Bottom-UI-Interaktion.
    - `gameplay_scene_persistence.cpp`: Save-Slots, Checkpoints und Fog-of-War.
    - `items/`: Item-spezifische Module wie `double_jump`.
    - `player/`: Entkoppelte Player-Submodule.
      - `player_logic.*`: Orchestriert das Frame-Update.
      - `movement_input.*`: Input -> horizontale Geschwindigkeit.
      - `jump_logic.*`: Sprung/Coyote-Time/Doppelsprung.
      - `collision_logic.*`: X/Y-Kollisionen inkl. Schraegkacheln.
      - `danger_logic.*`: Gefahrenkachelpruefung.
  - `menu/`: Menuesystem (Controller und Views).
    - `main_menu_layout.h`: Gemeinsame Geometrie fuer Touchbereiche und gezeichnete Buttons.
  - `ui/`: UI-nahe Komponenten.
  - `main.c`, `main.cpp`: Einstiege fuer Build/Runtime.
- `romfs/maps/`: Exportierte JSON-Maps aus dem LevelEditor; werden von Runtime und Simulator gelesen.
- `build/`: Build-Artefakte (inkl. Simulator-Binaries).
- `sim_saves/`: Simulationsnahe Save-Daten.
- `Makefile`: 3DS-Build, erwartet `DEVKITARM` und nutzt 3DS-Libraries.

Abhaengigkeiten:
- 3DS-Build: `devkitARM`, `libctru`, `citro2d`, `citro3d`, `tex3ds`, `make`.
- Datenabhaengigkeit: Aktuelle Maps aus `LevelEditor/level/*.lua` (via Export nach `romfs/maps/*.json`).

### `LevelEditor/` (Editor + Export)
- `main.lua`, `views.lua`, `views/`: UI und Editoransichten.
- `level/`: Quellformat der Karten (`*.lua`) inkl. `world.lua`.
- `export_maps.ps1`: Windows-Export von Lua -> JSON.
- `export_maps.py`: Plattformuebergreifender Export.
- `map_exporter.lua`: Lua-basierte Exportlogik.
- `run_editor.bat/.sh`: Startskripte fuer den Editor.

Abhaengigkeiten:
- Editor: `LÖVE`.
- Export: PowerShell (Windows) oder Python 3 (plattformuebergreifend).
- Zielsystem: Export schreibt nach `3ds-cpp/romfs/maps`.

### `simulator/` (Desktop-Testumgebung)
- `window_main_win32.cpp`: Fenster-Simulator (Win32).
- `main.cpp`: Konsolen-Simulator.
- `compat/`: Kompatibilitaetscode fuer Desktop-Umgebung.
- `run_simulator_window.bat/.sh`, `run_simulator_console.bat/.sh`: Build-/Startskripte.

Abhaengigkeiten:
- C++17-Compiler (`g++` oder `cl.exe`).
- Unter Windows fuer Fenster-Build: Win32-Systemlibs (z. B. `gdi32`, `winmm`).
- Quellcode-Abhaengigkeit: nutzt Module aus `3ds-cpp/source/core` und Menueteilen.

### `scripts/` (Checks + Setup)
- `check_german_comments.*`: prueft deutsche Kommentare in geaenderten Quelldateien.
- `check_wiki_links.*`: prueft Obsidian-Links und erkennt nicht verlinkte Doku-Seiten.
- `install_devkitarm.*`: Hilfen fuer devkitARM-Installation.
- `fix_bat_assoc.*`: repariert `.bat`-Dateizuordnung unter Windows.

Abhaengigkeiten:
- PowerShell fuer `.ps1`.
- Python 3 fuer Python-basierte Pruefskripte.
- devkitPro/MSYS2 fuer Installationsskripte.

### `docs/` (Wissensbasis)
- `workspace/`: Orientierung und Architekturuebersicht.
- `game/`, `simulator/`, `level-editor/`: Bereichsbezogene technische Doku.
- `tooling/`, `quickstart/`: Arbeitsprozesse und schnelle Einstiege.
- `GESAMTDOKU.md`: zentraler Hub.

Abhaengigkeiten:
- Konventionen aus [[docs/AI_REGELN|AI-Regeln]] (Deutsch, Wiki-Links, Querverweise).

## 3) Daten- und Abhaengigkeitsfluss (Kurzform)

1. `LevelEditor/level/*.lua` wird gepflegt.
2. Einstieg ueber `programme/export_level_maps.bat` oder `programme/build_game_from_editor.bat`.
3. Export erzeugt `3ds-cpp/romfs/maps/*.json`.
4. Runtime (`3ds-cpp/source`) und Simulator (`simulator/`) lesen dieselben JSON-Maps.
5. Skripte unter `scripts/` validieren Kommentare und Doku-Links lokal/CI.

## 4) Externe Workspace-Ordner (ausserhalb des Repos)

Diese Ordner gehoeren nicht zum Git-Repository `3ds-Metroid1`, sind aber im Workspace vorhanden:

| Ordner | Rolle im Gesamtsetup | Typische Abhaengigkeiten |
|---|---|---|
| `c:\devkitPro` | Toolchain fuer 3DS-Builds (`devkitARM`, Libraries, MSYS2) | Pacman/MSYS2, devkitPro-Pakete |
| `c:\Users\hoetting.y\Downloads\winlibs-...\mingw64` | Lokaler GCC/MinGW-Compiler fuer Windows-Simulator | MinGW-w64 Toolchain |
| `c:\Users\hoetting.y\Downloads\cpp3ds-master\cpp3ds-master` | Externe Bibliothek/Referenzprojekt im Workspace | CMake, 3DS-Toolchain (je nach Nutzung) |

Hinweis:
- Fuer den Kernworkflow dieses Repos sind vor allem `3ds-Metroid1`, `devkitPro` und ein lokaler C++-Compiler relevant.
- `cpp3ds-master` ist im aktuellen Repo-Flow nicht direkt als Pflichtabhaengigkeit verdrahtet.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/workspace/README|Workspace-Struktur]]
- [[docs/workspace/ARCHITEKTUR_UEBERSICHT|Architektur-Uebersicht]]
- [[docs/workspace/EINSTEIGER_GUIDE|Einsteiger-Guide]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
- [[docs/game/SETUP|Game Setup]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/level-editor/SETUP|LevelEditor Setup]]
