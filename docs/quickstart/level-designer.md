# Quick Start – Level Designer

## Ziel (5 Minuten)
- Level im Editor bearbeiten
- nach JSON exportieren
- im Simulator prüfen

## Schritte
1. Editor starten:
   - `LevelEditor\run_editor.bat`
2. Änderungen in `LevelEditor/level/*.lua` speichern.
3. Karten exportieren:
   - `LevelEditor\run_export_maps.bat`
4. Prüfen, ob JSONs in `3ds-cpp/romfs/maps` aktualisiert wurden.
5. Simulator starten:
   - `simulator\run_simulator_window.bat`

## Hinweise
- `world.lua` steuert Raumplatzierung/Verbindungen.
- Wenn Export fehlschlägt, zuerst Konsole auf Validierungsfehler prüfen.
- Der Simulator lädt Karten aus `3ds-cpp/romfs/maps`.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/level-editor/README|LevelEditor Überblick]]
- [[docs/level-editor/SETUP|LevelEditor Setup]]
- [[docs/simulator/README|Simulator Überblick]]
- [[docs/workspace/ARCHITEKTUR_UEBERSICHT|Gesamtarchitektur]]
