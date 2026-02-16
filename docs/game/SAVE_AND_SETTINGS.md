# Game: Save-, Slot- und Settings-System

## Save-Slots
- Anzahl: 3 (`SAVE_SLOT_COUNT`).
- Save-Dateien:
  - `sdmc:/3ds/3ds-cpp/savegame_slot1.dat`
  - `sdmc:/3ds/3ds-cpp/savegame_slot2.dat`
  - `sdmc:/3ds/3ds-cpp/savegame_slot3.dat`
- Legacy-Fallback für Slot 1:
  - `savegame.dat`

## Continue/Checkpoint-Verhalten
- Continue lädt den persistierten Checkpoint pro Slot.
- Checkpoints werden beim Betreten von Checkpoint-Zellen und bei Raumwechseln entsprechend aktualisiert.
- Beim Shutdown wird bevorzugt der letzte valide Checkpoint gespeichert (Fallback: aktuelle Position, falls kein Checkpoint gesetzt).

## Fog-of-War / Discovery
- Pro Slot getrennte Dateien:
  - `visited_slot1.dat`, `visited_slot2.dat`, `visited_slot3.dat`
- Kartenfortschritt kann im Hauptmenü pro aktivem Slot zurückgesetzt werden.

## Settings
- Datei: `sdmc:/3ds/3ds-cpp/settings.dat`
- Gespeicherte Werte:
  - `debugEnabled`
  - `controlsSwapped`
  - `showFps`

## Wichtige Methoden
- `GameplayScene::startNewGame(slot)`
- `GameplayScene::loadFromCheckpoint(slot)`
- `GameplayScene::hasPersistentSave(slot)`
- `GameplayScene::resetVisitedProgress(slot)`
- `AppSettings::load/save`
