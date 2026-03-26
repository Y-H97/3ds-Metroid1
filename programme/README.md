# Programme

Dieser Ordner ist der zentrale Einstiegspunkt fuer alle ausfuehrbaren Programme und Arbeitsablaeufe im Projekt.
Die eigentlichen Quellordner bleiben an ihrem fachlichen Platz, aber die Startpunkte liegen gebuendelt hier.

## Enthaltene Programme und Skripte

### LevelEditor
- `run_level_editor.bat`
  - Startet den Level-Editor auf Windows ueber den bestehenden Einstieg in `LevelEditor/`.
- `export_level_maps.bat`
  - Exportiert die Level aus `LevelEditor/level/` nach `3ds-cpp/romfs/maps/`.

### Simulator
- `run_simulator_window.bat`
  - Baut und startet den grafischen Windows-Simulator.

### Nintendo-3DS-Build
- `build_3ds.bat`
  - Baut die 3DS-Version mit den bereits exportierten Maps.
- `build_game_from_editor.bat`
  - Fuehrt den kompletten Ablauf in einem Schritt aus:
    1. Level exportieren
    2. 3DS-Build starten
    3. Ausgabedateien bereitstellen

## Warum dieser Ordner existiert

Frueher lagen die Startskripte auf mehrere Projektordner verteilt. Dadurch war fuer neue Mitarbeitende unklar,
welches Skript in welcher Reihenfolge ausgefuehrt werden muss. Dieser Ordner sammelt deshalb alle relevanten
Programme an einer Stelle und entkoppelt den Einstieg von der internen Ordnerstruktur.

## Typische Nutzung

### Nur den Editor starten
- `programme\run_level_editor.bat`

### Nur Maps exportieren
- `programme\export_level_maps.bat`

### Simulator starten
- `programme\run_simulator_window.bat`

### Kompletten 3DS-Workflow ausfuehren
- `programme\build_game_from_editor.bat`

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/PROGRAMS|Programme und Startskripte]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
- [[docs/game/SETUP|Game Setup]]
- [[docs/level-editor/SETUP|LevelEditor Setup]]